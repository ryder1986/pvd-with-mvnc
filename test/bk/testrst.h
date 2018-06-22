
//frame input:  1.enter_ppl_num 2.out_ppl_num 3.count
//frame output: 1. count 2.sum 3.busy_time
class camera{
public:
    void check()
    {
        process(in_count,out_count,count);
        flow_sum+=in_count-out_count;

    }
    int count;
    int busy_time;
    int flow_sum;

    int busy_start_time;
    int busy_start_count;
    int now;


    int last_in_count;

    bool busy;
    bool last_busy;


    //busy start: last frame no result count&&this frame have in_count
    //busy stop: this frame have out_come &&



};
