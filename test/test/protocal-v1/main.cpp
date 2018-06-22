
#include<iostream>
#include <vector>
#include <cstdint>
#include<iomanip>
#include <cstring>
using namespace std;

#define DATA_START 0x7e//1
#define ID 0x01//2
//#define ID 0x01//2
//#define ID 0x7d//2
#define VER 0x01//3
#define OP 0x61//4
#define CLASS 0x1f//5
//6 data
//7 check
#define DATA_END 0x7e//8
#define DATA_START 0x7e//

void encode_prot( vector <uint8_t> &bs)
{
    bs.push_back(DATA_START);
    bs.push_back(ID);
    bs.push_back(VER);
    bs.push_back(OP);
    bs.push_back(CLASS);
    //data
    int num=2;
    bs.push_back(0x01);//direction
    bs.push_back(num);//channel total count
    for(int i=0;i<num;i++){
        bs.push_back(17);//area id
        bs.push_back(1);//exist
        bs.push_back(30);//percent
        bs.push_back(2);//busy
        bs.push_back(1);//valid
    }
    uint8_t check=0;

    if(bs.size()>1){
        check=bs[0];
        for(int i=1;i<bs.size();i++){
            check^=bs[i];
        }
    }
    bs.push_back(check);
    bs.push_back(DATA_END);


    if(bs.size()>1){

        vector <uint8_t>::iterator it=bs.begin()+1;
        while(it!=bs.end()-1){
            if(*it==0x7E)
            {
               *it=0x7D;
                bs.insert(it+1,1,(uint8_t)0x5E);

            }else
            if(*it==0x7D)
            {
              *it=0x7D;

                bs.insert(it+1,1,(uint8_t)0x5D);

            }
            it++;
        }

//        for(int i=1;i<bs.size()-1;i++){
//            if(bs[i]==0x7E)
//            {
//                bs[i]=0x7D;
//                bs.insert(i,1,(uint8_t)0x5E);
//                i++;
//            }
//            if(bs[i]==0x7D)
//            {
//                bs[i]=0x7D;
//                bs.insert(i,1,(uint8_t)0x5D);
//                i++;
//            }
//        }
    }


    //  cout<<hex<<c<<endl;
}

typedef struct cdata_type{
    uint8_t no;
    uint8_t exist;
    uint8_t percent;//zhan you lv
    uint8_t busy_state;// 1-5 , judge by percent
    uint8_t valid;
}cdata_t;

typedef struct data_type{
    uint8_t direction;
    uint8_t channel_count;
    vector <cdata_t> channels;

}data_t;

vector <uint8_t>  process_protocal(data_t data )
{
    vector <uint8_t> bs;
    bs.push_back(DATA_START);
    bs.push_back(ID);
    bs.push_back(VER);
    bs.push_back(OP);
    bs.push_back(CLASS);
    //data
    int num=2;
    bs.push_back(0x01);//direction
    bs.push_back(num);//channel total count
    for(int i=0;i<num;i++){
        bs.push_back(17);//area id
        bs.push_back(1);//exist
        bs.push_back(30);//percent
        bs.push_back(2);//busy
        bs.push_back(1);//valid
    }
    uint8_t check=0;

    if(bs.size()>1){
        check=bs[0];
        for(int i=1;i<bs.size();i++){
            check^=bs[i];
        }
    }
    bs.push_back(check);
    bs.push_back(DATA_END);


    if(bs.size()>1){

        vector <uint8_t>::iterator it=bs.begin()+1;
        while(it!=bs.end()-1){
            if(*it==0x7E)
            {
               *it=0x7D;
                bs.insert(it+1,1,(uint8_t)0x5E);

            }else
            if(*it==0x7D)
            {
              *it=0x7D;

                bs.insert(it+1,1,(uint8_t)0x5D);

            }
            it++;
        }
    }
    return bs;
}


int main()
{
    vector <uint8_t> bs;
    encode_prot(bs);

    //bs.push_back(0);
    uint8_t buf[100];
    if(bs.size()>100){
        cout << "out of range"<<endl;
    }
    else{
        memset(buf,0,100);

        for(int i=0;i<bs.size();i++)
        {
            buf[i]=bs[i];
        }
    }
    //    setiosflags(ios::showbase);
    //  ;
    for(int i=0;i<bs.size();i++)
    {
        cout<<hex<<(int)bs[i]<<' ';
    }
    cout<< endl;
    //   cout<<   setbase(16)<<(int)buf[0]<<(int)buf[1]<<endl;
    return 1;

}
