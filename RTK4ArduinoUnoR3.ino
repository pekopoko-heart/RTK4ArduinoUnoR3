///////////////////////////////////////////////
// RTKernel for Arduino UNO R3
///////////////////////////////////////////////

// タスクの状態定義
#define STOP	  0	  // 停止状態
#define RUN	    1	  // 実行状態
#define READY	  2	  // 実行可能状態
#define SUSPEND	3	  // 待ち状態
#define NULL	0

/////////////////////////
//  タスクコントロールブロック
/////////////////////////

struct TCB {
    unsigned char	status;		// タスクの状態
    void	(*task)(void);	  // タスクのエントリーアドレス
    unsigned char	no;		    // タスク定義番号
    unsigned char	level;		// タスクの優先順位 大きい方が優先順位が高い
    struct	TCB* next;	    // TCB リンク
};

struct SCB {
    struct TCB* run;		    // RUN タスクのTCB アドレス
    struct TCB* ready;		  // 最優先READY タスクの TCB アドレス 
    struct TCB* suspend;	  // 最優先 SUSPEND タスクの TCB アドレス
};

// 管理データ定義
struct TCB tcb[TASK_MAX];		          // タスクコントロールブロック 
struct SCB scb = { NULL,NULL,NULL };	// スケジューラコントロールブロック
void (*task_entry)(void);		          // タスクエントリアドレス

// 手続き
void task_sw(unsigned char no);			  // タスク起動要求
void task_create(void(*task)(void), unsigned char id, unsigned char level); // タスク生成
void reschedule(void);			          // タスク終了時リスケジューリング処理
void link_ready(unsigned char no);		// TCBのレディリンクへの接続処理
void link_suspend(unsigned char no);	// TCBのサスペンドリンクへの接続処理
unsigned char  get_ready(void);			  // レディリンクからのTCB の獲得処理
unsigned char  get_suspend(void);			// サスペンドリンクからのTCBの獲得処理
void dispatch(void (*task)(void));		// コンテキストスイッチ処理

///////////////////////////////////////////////
//  タスク切り替え処理
// 
//    1) dispatch()のスタックフレームや退避レジスタの後処理のため、起動するタスクの処理完了後に一旦 
// 　　　postprocess に戻ってくるように、postprocess のアドレスをスタックに積んでおく。 
//    2) 起動するタスク終了時にリスケジューリング処理を行うため、reschedule() のエントリーアドレスを
//       スタックに積み、タスクからのリターンにより自動的にリスケジューリングを行わせる。
//    3) 起動タスクのエントリーアドレスをスタックに積み、reti命令で割り込み許可状態でタスクを起動する。
//
//    このRTKernelでは、機能限定（実行中タスクのイベント待ちなどのWAIT状態を扱わない）と、Cの関数を
//    タスクに定義することにより、コンテキストスイッチ時のスタックの切り換え及びレジスタの退避を不要に
//    している。
///////////////////////////////////////////////

void dispatch(void (*task_entry)(void))
{  
    // reschedule()のアドレスを取得
    unsigned int addr = (unsigned int)reschedule;
    
    asm volatile (

      // ラベルのアドレスはByteアドレスのため1ビットシフトでプログラムアドレスに修正
      " ldi r24, pm_lo8(postprocess) \n"  // 戻りアドレスの下位バイトをr24にロード
      " push r24 \n"          // スタックにPUSH
      " ldi r24, pm_hi8(postprocess) \n"  // 戻りアドレスの上位バイトをr24にロード
      " push r24 \n"          // スタックにPUSH

      " ldi r24, lo8(%0) \n"  // reschedule()のアドレスの下位バイトをr24にロード
      " push r24 \n"          // スタックにPUSH
      " ldi r24, hi8(%0) \n"  // reschedule()のアドレスの上位バイトをr24にロード
      " push r24 \n"          // スタックにPUSH

      " mov r24, %A1 \n"      // task_entry()のアドレスの下位バイトをr24にロード
      " push r24 \n"          // スタックにPUSH
      " mov r24, %B1 \n"      // task_entry()のアドレスの上位バイトをr24にロード
      " push r24 \n"          // スタックにPUSH

      // 割り込みを許可してnext task起動      
      " reti \n"

      " postprocess: \n"

        :                 // 出力オペランドリスト
        : "i" (addr) , "r" (task_entry)    // 入力オペランドリスト(%An下位バイト %Bn上位バイト) 
        : "r24"           //クラバー:使用レジスタをコンパイラに明示
    );
}

///////////////////////////////////////////////
//  タスクの起動要求
//    タスク定義番号で指定されるタスクの起動要求を、スケジューラに対
//    して行う。スケジューラは、優先順位により､ CPUを割り付けるタスク
//    を選択し、そのタスクに制御を移す。
///////////////////////////////////////////////

void task_sw(unsigned char no)
{
    // タスクスケジューリング中は、割り込み禁止 */
    noInterrupts();

    /////////////////////////
    // タスクスケジューリング
    /////////////////////////

    // 起動済みTASKに対する再起動要求は無効。割り込みを許可してリターン
    if (tcb[no].status != STOP) {
        interrupts();
        return;

    // 起動中TASKが存在しない場合
    }
    else if (scb.run == NULL) {

        // 起動要求TASKをRUNにする
        tcb[no].status = RUN;
        scb.run = &tcb[no];
        tcb[no].next = NULL;

        // タスクスイッチ
        // タスクは、フラグレジスタの設定により、割り込み許可状態で起動する。
        dispatch(tcb[no].task);
        return;

        // 起動要求TASKの優先順位がRUN TASK の優先順位よりも高い場合、タスクスイッチを実行する。
    }
    else if ((*scb.run).level < tcb[no].level) {

        // 実行中TASKはSUSPEND LINK につなげる
        (*scb.run).status = SUSPEND;
        link_suspend((*scb.run).no);

        // 起動要求TASKをRUNにする
        tcb[no].status = RUN;
        scb.run = &tcb[no];
        tcb[no].next = NULL;

        // タスクスイッチ
        // タスクは、フラグレジスタの設定により、割り込み許可状態で起動する。
        dispatch(tcb[no].task);
        return;

        // 起動要求TASKのpriority が RUN TASK の priorityよりも低い場合、起動要求TASK を 
        // READY LINKにつなげ、 RUN TASK の処理を続行する。
    }
    else {

        // 起動要求TASK のステータスをREADYとし、READY LINK につなげる
        tcb[no].status = READY;
        link_ready(no);

        // 割り込みを許可してリターン
        interrupts();
        return;
    }
}

///////////////////////////////////////////////
//  タスク終了時リスケジューリング処理
//    タスクの実行終了時に起動され、次に起動するタスクを、レディリンクと、
//    サスペンドリンクから優先順位により選択し、そのタスクに制御を移す。
///////////////////////////////////////////////

void reschedule(void)
{
    unsigned char no;

    // タスクスケジューリング中は、割り込み禁止
    noInterrupts();

    /////////////////////////
    // RUN TASK の終了
    /////////////////////////
    (*scb.run).status = STOP;
    scb.run = NULL;

    /////////////////////////
    // タスクスケジューリング
    /////////////////////////

    // SUSPENS TASK、READY TASKともに存在しない場合、割り込みを許可してリターン
    if (scb.suspend == NULL && scb.ready == NULL) {
        interrupts();
        return;

    // SUSPENS TASKが存在せず、READY TASK のみ存在する場合、
    // または、READY TASKの優先順位がSUSPEND TASK の優先順位
    // よりも高い場合、タスクスイッチを実行し、READY TASKを起動する。
    }
    else if ((scb.ready != NULL && scb.suspend == NULL)
        || (scb.ready != NULL && (*scb.ready).level > (*scb.suspend).level)) {

        // READY TASK を READY LINK から取り出す
        no = get_ready();

        // READY TASKをRUNにする
        tcb[no].status = RUN;
        scb.run = &tcb[no];
        tcb[no].next = NULL;

        // タスクスイッチ
        // タスクは、フラグレジスタの設定により、割り込み許可状態で起動する。
        dispatch(tcb[no].task);
        return;

    }
    else {

        // READY TASKが存在しない場合、または、SUSPEND TASKの
        // 優先順位がREADY TASK の優先順位よりも高い場合､ SUSPEND
        // TASKの実行に戻る。

        // SUSPEND TASKをSUSPEND LINKから取り出す
        no = get_suspend();

        // SUSPEND TASK を RUNにする
        tcb[no].status = RUN;
        scb.run = &tcb[no];
        tcb[no].next = NULL;

        // 割り込みを許可して、SUSPEND TASK の実行に戻る
        interrupts();
        return;
    }
}

///////////////////////////////////////////////
//  タスクの生成
//    タスクのエントリーアドレス、タスク定義番号、タスクの優先順位を
//    TCBに登録し、タスクを定義する。
///////////////////////////////////////////////

void task_create(void(*task) (void), unsigned char no, unsigned char level)
{
    tcb[no].status = STOP;
    tcb[no].task = task;
    tcb[no].next = NULL;
    tcb[no].level = level;
    tcb[no].no = no;
}

///////////////////////////////////////////////
//  TCBのレディリンクへの接続処理
//    タスク定義ナンバーで指定されるタスクの TCB ポインタを、タスク
//    の優先順位の順にリンクされた TCBのレディリンクにつなげる。
///////////////////////////////////////////////

void link_ready(unsigned char no)
{
    struct TCB *tcb1, *tcb2;

    // READY TASKが存在しない場合、タスク定義ナンバーで指定さ
    // れるタスクのTCBをレディリンクの先頭(スケジューラ コントロールブロック
    // のレディリンクポインタ)につなぐ
    if (scb.ready == NULL) {
        scb.ready = &tcb[no];
        tcb[no].next = NULL;

    // タスク定義ナンバーで指定されるタスクの優先順位が、レディリンクの
    // 先頭(スケジューラコントロールブロックのレディリンクポインタ) のタスクの
    // 優先順位よりも高い場合レディリンクの先頭につなぐ。
    }
    else if ((*scb.ready).level < tcb[no].level) {

        tcb[no].next = scb.ready;
        scb.ready = &tcb[no];

    // レディリンクに、タスク定義ナンバーで指定されるタスクの TCBが、タスクの
    // 優先順位の順に並ぶように挿入する。
    }
    else {

        tcb2 = scb.ready;
        do {
            tcb1 = tcb2;
            tcb2 = (*tcb1).next;

            // TCBのリンクポインタがNULL の場合、そのTCBがレディリンクの最終TCBである。
            // この場合、タスク定義ナンバーで指定されるタスクの TCBをレディリンクの最後に挿入する。
            if (tcb2 == NULL) {
                (*tcb1).next = &tcb[no];
                tcb[no].next = NULL;
                return;
            }

            // TCBのレディリンク間のタスクの優先順位をチェック
        } while ((*tcb2).level > tcb[no].level);
        (*tcb1).next = &tcb[no];
        tcb[no].next = tcb2;
    }
}

///////////////////////////////////////////////
//  TCBのサスペンドリンクへの接続処理
//    タスク定義ナンバーで指定されるタスクのTCB ポインタを、タスク
//    の優先順位の順にリンクされた TCBのサスペンドリンクにつなげる。
///////////////////////////////////////////////

void link_suspend(unsigned char no)
{
    struct TCB *tcb1, *tcb2;

    // SUSPEND TASKが存在しない場合、タスク定義ナンバーで指定される
    // タスクのTCBをサスペンドリンクの先頭 (スケジューラコントロール
    // ブロックのサスペンドリンクポインタ) につなぐ
    if (scb.suspend == NULL) {
        scb.suspend = &tcb[no];
        tcb[no].next = NULL;

    // タスク定義ナンバーで指定されるタスクの優先順位が、サスペンドリンクの
    // 先頭 (スケジューラコントロールブロックのサスペンドリンクポインタ) の
    // タスクの優先順位よりも高い場合、サスペンドリンクの先頭につなぐ。
    }
    else if ((*scb.suspend).level < tcb[no].level) {
        tcb[no].next = scb.suspend;
        scb.suspend = &tcb[no];

    // サスペンドリンクに、タスク定義ナンバーで指定されるタスクのTCBが、 
    // タスクの優先順位の順に並ぶように挿入する
    }
    else {
        tcb2 = scb.suspend;
        do {
            tcb1 = tcb2;
            tcb2 = (*tcb1).next;

            // TCBのリンクポインタがNULLの場合、そのTCBがサスペンドリンクの最終TCB。
            // この場合、タスク定義ナンバーで指定されるタスクのTCBをサスペンドリンクの最後に挿入する。
            if (tcb2 == NULL) {
                (*tcb1).next = &tcb[no];
                tcb[no].next = NULL;
                return;
            }

            // TCBのサスペンドリンク間のタスクの優先順位をチェック
        } while ((*tcb2).level > tcb[no].level);
        (*tcb1).next = &tcb[no];
        tcb[no].next = tcb2;
    }
}

///////////////////////////////////////////////
// レディリンクからのTCBの獲得処理
//    タスクの優先順位の順にリンクされたTCBのレディリンクから、
//    先頭のTCBを取り出し、そのTCBで定義しているタスク定義番号を返す。
///////////////////////////////////////////////

unsigned char get_ready(void)
{
    unsigned char no;

    // READY TASKが存在しない場合、NULLを返す。
    if (scb.ready == NULL) {
        return(NULL);   // something happened

    // TCBのレディリンクの先頭のTCBを取り出し、そのTCBで定義しているタスク定義番号を返す。
    }
    else {
        no = (*scb.ready).no;
        scb.ready = (*scb.ready).next;
        return(no);
    }
}

///////////////////////////////////////////////
// サスペンドリンクからのTCBの獲得処理
//    タスクの優先順位順にリンクされたTCBのサスベンドリンクから、
//    先頭のTCBを取り出し、そのTCBで定義しているタスク定義番号を返す。
///////////////////////////////////////////////

unsigned char get_suspend(void)
{
    unsigned char no;

    // SUSPEND TASKが存在しない場合、NULLを返す。
    if (scb.suspend == NULL) {
        return(NULL);   // something happened

    // TCBのサスペンドリンクの先頭のTCBを取り出し、そのTCBで定義しているタスク定義番号を返す。
    }
    else {
        no = (*scb.suspend).no;
        scb.suspend = (*scb.suspend).next;
        return(no);
    }
}
