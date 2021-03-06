#include "participant.h"
#define MAXDATASIZE 100000
participant::participant(Socket pa_,Socket co_)
{
    socketInfo=pa_;
    CoInfo=co_;
    cout<<"create a participant "<<socketInfo.IP<<":"<<socketInfo.port<<endl;
    cout<<"coordinator "<<CoInfo.IP<<":"<<CoInfo.port<<endl;
    pa_state=WAIT;
    recvFromCoorinator();
}
MSG participant::recvFromCoorinator()
{
    int pa_sock=SocketApi::Socket();
    SocketApi::Bind(pa_sock,socketInfo.port);
    SocketApi::Listen(pa_sock);
    while(1)
	{
       std::string peer_ip;
       int peer_port;
       int sock = SocketApi::Accept(pa_sock,peer_ip,peer_port);   
       std::cout << "part_sock :" << sock <<std::endl;
       if(sock>= 0)
       {
       	  std::cout << peer_ip << " : " << peer_port <<std::endl;
          Connect* part_conn = new Connect(pa_sock);
          string line;
          part_conn->RecvLine(sock,line);
          cout<<"participant recv and finish the request:"<<MsgAnalyze(line).message<<endl;
       }
	}
}
MSG participant::MsgAnalyze(string resp)
//处理resp
{
    MSG resultMSG={.state=false,.message="-ERROR\r\n"};//初始化
    vector<string> result=mysplit(resp,"\r\n");
    // for(int i=0;i<result.size();i++)
    // {
    //     cout<<result[i]<<endl;
    // }
    if(result[2]=="GET")
    {
        string key=result[4];
        return get(key);
    }
    else if(result[2]=="SET")
    {
        string key=result[4];
        string value="";
        for(int i=6;i<result.size();i+=2)
        {
            if(i!=6) value=value+" ";
            value=value+result[i];
        }
        if(value=="") return resultMSG;
        return set(key,value);
    }
    else if(result[2]=="DEL")
    {
        vector<string> keyList;
        for(int i=4;i<result.size();i+=2)
        {
            keyList.push_back(result[i]);
        }
        return delate(keyList);
    }
    else return resultMSG;


}
MSG participant::set(string key,string value)//对本地数据库进行set操作
{
    MSG resultMSG={.state=false,.message="-ERROR\r\n"};//初始化
    pair< std::map< string,string >::iterator,bool > ret;
    ret=database.insert(pair<string,string>(key,value));
    if(ret.second)
    {
        resultMSG.state=true;
        resultMSG.message="+OK\r\n";//成功。
        return resultMSG;
    }
    return errorMSG;
}
MSG participant::delate(vector<string> keyList)
//对本地数据库进行delate操作
{
    MSG resultMSG={.state=false,.message="-ERROR\r\n"};//初始化
    int delNum=0;
    int len=keyList.size();
    for(int i=0;i<len;i++)
    {
        map<string,string>::iterator iter=database.find(keyList[i]);
       if(iter!=database.end())//成功删除
       {
           delNum++;
       }
    }
    string msg=":"+to_string(delNum)+"\r\n";
    resultMSG.message=msg;
    return resultMSG;
}
MSG participant::get(string key)
//对本地数据进行get操作.get value of key
{
    MSG resultMSG={.state=false,.message="-ERROR\r\n"};//初始化

    map<string,string>::iterator iter=database.find(key);
    if(iter!=database.end())//get到了
    {
        string value=iter->second;
        vector<string> result=mysplit(value," ");
        string msg='*'+to_string(result.size());
        // for(int i=0;i<result.size();i++)
        // {
        //     cout<<result[i]<<endl;
        // }
        for(int i=0;i<result.size();i++)
        {
            int len=result[i].length();
            msg=msg+"\r\n$"+to_string(len)+"\r\n"+result[i];
        }
        msg=msg+"\r\n";
        resultMSG.message=msg;
    }
    return resultMSG;
}
bool participant::logwriter(string data)
{
    FILE* fp_ = NULL;
    char *buf_;
    char* p_;
    fp_ = fopen("/home/guolab/bowei/log/tf_debug_grpc.log", "w+");
    if (!fp_) {
      fprintf(fp_, "Can't open target file.\n");
    }
    int file_size = p_ - buf_;
    #define BLOCK_SIZE 1024
    int i;
    for (i = 0; i < file_size/BLOCK_SIZE; i++) {
        fwrite(buf_ + i*BLOCK_SIZE, sizeof(char), BLOCK_SIZE, fp_);
    }
    int remainder = file_size - (BLOCK_SIZE * i);
    fwrite(buf_ + i*BLOCK_SIZE, sizeof(char), remainder, fp_);
    
    free(buf_);
    fclose(fp_);
}
bool participant::recovery()
{

}
