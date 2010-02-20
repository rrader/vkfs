#include <iostream>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <stdlib.h>
#include <sstream>

#include "vklib.h"

namespace vklib
{

using namespace cURLpp;
using namespace Options;
using namespace types;
using namespace std;

std::string RequestAnswer;

string IntToStr(int i)
{
    string str;
    stringstream stream;
    stream << i;
    //str = ;
    return stream.str();
}

size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb)
{
    RequestAnswer += ptr;
    int FullSize = size*nmemb;
    return FullSize;
}

int VKObject::Login(std::string EMail,std::string Passwd)
{
    Easy* request=new Easy;
    WriteFunctionFunctor functor(WriteMemoryCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    request->setOpt(cb);
    request->setOpt(Url("http://login.userapi.com/auth"));
    request->setOpt(Post(true));

    request->setOpt(PostFields("login=force&site=5323&email="+EMail+"&pass="+Passwd));

    request->setOpt(Header(true));
    request->setOpt(MaxRedirs(0));
    request->perform();
    delete request;
    int is=RequestAnswer.find("Location: ");
    is=RequestAnswer.find("#",is+1)+3;
    int i=is;
    sid="";
    while ((RequestAnswer[i]!='\xD')and(RequestAnswer[i]!='\xA'))
        sid+=RequestAnswer[i++];
    return 0;
}

int VKWallReader::ReadWall(VKObject& session,int uid,int from, int to)
{
    Easy* wall=new Easy;
    WriteFunctionFunctor functor(WriteMemoryCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    wall->setOpt(cb);
    wall->setOpt(Url("http://userapi.com/data"));
    wall->setOpt(Post(true));
    wall->setOpt(Header(false));

    std::string fields=session.sid+"&act=wall&id="+IntToStr(uid)+"&from="+IntToStr(from)+"&to="+IntToStr(to);

    wall->setOpt(PostFields(fields));
    RequestAnswer="";
    wall->perform();
    std::stringstream stream(RequestAnswer);
    json::Reader::Read(jsonresponse, stream);
    delete wall;
}

int VKWallReader::MessageCount()
{
    return ((json::Number&)(jsonresponse["n"])).Value();
}

int VKWallReader::GetMessageID(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][0])).Value();
}

int VKWallReader::GetMessageTime(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][1])).Value();
}

string VKWallReader::GetMessageText(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::String&)(d[n][2][0])).Value();
}

int VKWallReader::GetMessageType(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][2][1])).Value();
}

int VKWallReader::GetMessageSenderID(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][3][0])).Value();
}

string VKWallReader::GetMessageSenderName(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::String&)(d[n][3][1])).Value();
}

}
