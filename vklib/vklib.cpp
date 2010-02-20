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
    RequestAnswer="";
    request->perform();
    delete request;
    int is=RequestAnswer.find("Location: ");
    is=RequestAnswer.find("#",is+1)+3;
    int i=is;
    sid="";
    while ((RequestAnswer[i]!='\xD')and(RequestAnswer[i]!='\xA'))
        sid+=RequestAnswer[i++];
    is=RequestAnswer.find("remixpassword");
    is=RequestAnswer.find("=",is+1)+1;
    remixpassword="";
    i=is;
    while (RequestAnswer[i]!=';')
        remixpassword+=RequestAnswer[i++];
    return 0;
}

int CheckResponse(VKObject& session,std::string rp)
{
    if (strcmp(rp.c_str(),"{\"ok\":-1}")==0)
    {
        Easy* request=new Easy;
        WriteFunctionFunctor functor(WriteMemoryCallback);
        WriteFunction* cb = new curlpp::options::WriteFunction(functor);
        request->setOpt(cb);
        request->setOpt(Url("http://login.userapi.com/auth"));
        request->setOpt(Post(true));

        string params="login=auto&site=5323&remixpassword="+session.remixpassword;
        request->setOpt(PostFields(params));

        request->setOpt(Header(true));
        request->setOpt(MaxRedirs(0));
        RequestAnswer="";
        request->perform();
        delete request;
        int is=RequestAnswer.find("Location: ");
        is=RequestAnswer.find("#",is+1)+3;
        int i=is;
        session.sid="";
        while ((RequestAnswer[i]!='\xD')and(RequestAnswer[i]!='\xA')and(RequestAnswer[i]!='\n'))
            session.sid+=RequestAnswer[i++];
        return 1;
    }
    return 0;
}

inline std::string replacestr(std::string text, std::string s, std::string d)
{
  for(unsigned index=0; index=text.find(s, index), index!=std::string::npos;)
  {
    text.replace(index, s.length(), d);
    index+=d.length();
  }
  return text;
}

int VKObject::RetrievePersonalInfo()
{
    Easy* request=new Easy;
    WriteFunctionFunctor functor(WriteMemoryCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    request->setOpt(cb);
    request->setOpt(Url("http://userapi.com/data"));
    request->setOpt(Post(true));

    request->setOpt(PostFields(sid+"&act=profile"));

    request->setOpt(Header(false));
    request->setOpt(MaxRedirs(0));
    RequestAnswer="";
    request->perform();
    delete request;
    if (CheckResponse(*this,RequestAnswer)!=0)
    {
        return RetrievePersonalInfo();
    }
    int i=RequestAnswer.size();


    while (RequestAnswer[i--]!='}');
    RequestAnswer.erase(i+2,RequestAnswer.size()-i);
    RequestAnswer=replacestr(RequestAnswer,"\\/","/");
    std::stringstream stream(RequestAnswer);
    profile.Clear();
    json::Reader::Read(profile, stream);
    return 0;
}

int VKObject::GetVkontakteID()
{
    return ((json::Number&)(profile["id"])).Value();
}

string VKObject::GetFirstName()
{
    return ((json::String&)(profile["fn"])).Value();
}

string VKObject::GetLastName()
{
    return ((json::String&)(profile["ln"])).Value();
}

string VKObject::GetStatus()
{
    return ((json::String&)(profile["actv"]["5"])).Value();
}

int VKObject::GetCountryID()
{
    return ((json::Number&)(profile["ht"]["coi"])).Value();
}

string VKObject::GetCountryName()
{
    return ((json::String&)(profile["ht"]["con"])).Value();
}

string VKObject::GetBirdthCityName()
{
    return ((json::String&)(profile["by"])).Value();
}

int VKObject::GetCityID()
{
    return ((json::Number&)(profile["ht"]["cii"])).Value();
}

int VKObject::GetMaritalStatus()
{
    return ((json::Number&)(profile["fs"])).Value();
}

int VKObject::GetPoliticalStatus()
{
    return ((json::Number&)(profile["pv"])).Value();
}

int VKObject::GetUserBirdthDay()
{
    return ((json::Number&)(profile["bd"])).Value();
}

int VKObject::GetUserBirdthMonth()
{
    return ((json::Number&)(profile["bm"])).Value();
}

int VKObject::GetUserBirdthYear()
{
    return ((json::Number&)(profile["by"])).Value();
}

string VKObject::GetCityName()
{
    return ((json::String&)(profile["ht"]["cin"])).Value();
}

string VKObject::GetMiddleName()
{
    return ((json::String&)(profile["mn"])).Value();
}

int VKWallReader::RetrieveWall(VKObject& session,int uid,int from, int to)
{
    sess=&session;
    Easy* wall=new Easy;
    WriteFunctionFunctor functor(WriteMemoryCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    wall->setOpt(cb);
    wall->setOpt(Url("http://userapi.com/data"));
    wall->setOpt(Post(true));
    wall->setOpt(Header(false));

    std::string fields=session.sid+"&act=wall&";
    if (uid!=0)
    {
        fields+="id="+IntToStr(uid)+"&";
    }
    fields+="from="+IntToStr(from)+"&to="+IntToStr(to);

    wall->setOpt(PostFields(fields));
    RequestAnswer="";
    wall->perform();
    if (CheckResponse(*sess,RequestAnswer)!=0)
    {
        return RetrieveWall(session,uid,from,to);
    }
    std::stringstream stream(RequestAnswer);
    jsonresponse.Clear();
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
