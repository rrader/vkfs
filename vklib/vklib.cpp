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

std::string IntToStr(int i)
{
    std::string str;
    std::stringstream stream;
    stream << i;
    //str = ;
    return stream.str();
}

int StrToInt (const std::string &str)
{
    std::stringstream ss(str);
    int n;
    ss >> n;
    return n;
}


using namespace cURLpp;
using namespace Options;
using namespace types;
using namespace std;

std::string RequestAnswer;
void* m_pBuffer;
int m_BufferSize;

size_t WriteStringCallback(char* ptr, size_t size, size_t nmemb)
{
    RequestAnswer += ptr;
    int FullSize = size*nmemb;
    return FullSize;
}

size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb)
{
    size_t realsize = size * nmemb;
    m_pBuffer = (char*) realloc(m_pBuffer, m_BufferSize + realsize);

    if (m_pBuffer == NULL) {
        realsize = 0;
    }

    memcpy((m_pBuffer+m_BufferSize), ptr, realsize);
    m_BufferSize += realsize;
    return realsize;
};

int VKObject::Login(std::string EMail,std::string Passwd)
{
    Easy* request=new Easy;
    WriteFunctionFunctor functor(WriteStringCallback);
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
        WriteFunctionFunctor functor(WriteStringCallback);
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

int GetURLFileSize(vector<FileCacheStruct>* CachedFiles,string URL)
{
    if (CachedFiles!=NULL)
    {
        vector<FileCacheStruct>::iterator it;
        it = CachedFiles->begin();
        while( it != CachedFiles->end() )
        {
            if (strcmp(it->url.c_str(),URL.c_str())==0)
            {
                if (time (NULL)-it->time<600)
                {
                    return it->size;
                }else
                {
                    delete it->ptr;
                    CachedFiles->erase(it);
                    break;
                }
            }
            it++;
        }
    }
    Easy* request=new Easy;
    WriteFunctionFunctor functor(WriteStringCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    request->setOpt(cb);
    request->setOpt(Url(URL));
    request->setOpt(HttpGet(true));
    request->setOpt(NoBody(true));
    request->setOpt(Header(true));
    request->setOpt(MaxRedirs(0));
    RequestAnswer="";
    request->perform();
    delete request;

    int is=RequestAnswer.find("Content-Length:");
    is=RequestAnswer.find(" ",is+1)+1;
    int i=is;
    string x="";
    while ((RequestAnswer[i]!='\xD')and(RequestAnswer[i]!='\xA')and(RequestAnswer[i]!='\n'))
        x+=RequestAnswer[i++];

    if (CachedFiles!=NULL)
    {
        FileCacheStruct fcs;
        fcs.ptr=NULL;
        fcs.time=time(NULL);
        fcs.url=URL;
        fcs.size=StrToInt(x);
        CachedFiles->push_back(fcs);
    }
    return StrToInt(x);
}

int VKObject::GetAvatarSize()
{
    return GetURLFileSize(&CachedFiles,GetProfileImagePath());
}

void RetrieveURL(vector<FileCacheStruct>* CachedFiles,string url, void*& buff, int& size)
{
    string u="echo ";
    u+=url+">> /home/roma/curl.txt";
    system(u.c_str());
    if (CachedFiles!=NULL)
    {
        vector<FileCacheStruct>::iterator it;
        it = CachedFiles->begin();
        while( it != CachedFiles->end() )
        {
            if (it->url.compare(url)==0)
            {
                    string u="echo \"Cached:";
                    u+=it->url+"\" >> /home/roma/curl.txt";
                    system(u.c_str());
                if (time (NULL)-it->time<600)
                {
                    if (it->ptr!=NULL)
                    {
                        size=it->size;
                        buff=it->ptr;
                        return;
                    }else
                        break;
                }else
                {
                    delete it->ptr;
                    CachedFiles->erase(it);
                    break;
                }
            }
            it++;
        }
        ;
    }
    Easy* request=new Easy;
    WriteFunctionFunctor functor(WriteMemoryCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    request->setOpt(cb);
    request->setOpt(Url(url));
    request->setOpt(HttpGet(true));
    request->setOpt(Header(false));
    request->setOpt(MaxRedirs(0));
    m_BufferSize=0;
    m_pBuffer=NULL;
    request->perform();
    delete request;

    size=m_BufferSize;
    buff=m_pBuffer;
    if (CachedFiles!=NULL)
    {
        FileCacheStruct fcs;
        fcs.ptr=buff;
        fcs.time=time(NULL);
        fcs.url=url;
        fcs.size=size;
        CachedFiles->push_back(fcs);
    }
}

int VKObject::RetreiveAvatar()
{
    RetrieveURL(&CachedFiles,GetProfileImagePath(),avatar,avatarsize);
    return avatarsize;
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

int VKObject::GetNPhotoSize(int n)
{
    return GetURLFileSize(&CachedFiles,GetNPhotoURL(n));
}

int VKObject::GetNMiniPhotoSize(int n)
{
    return GetURLFileSize(&CachedFiles,GetNMiniPhotoURL(n));
}

int VKObject::RetrievePersonalInfo()
{
    Easy* request=new Easy;
    WriteFunctionFunctor functor(WriteStringCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    request->setOpt(cb);
    request->setOpt(Url("http://userapi.com/data"));
    request->setOpt(Post(true));

    request->setOpt(PostFields(sid+"&act=profile"));

    request->setOpt(Header(false));
    request->setOpt(MaxRedirs(0));
    RequestAnswer="";
    request->perform();
    time_t x=time(NULL);
    while (time(NULL)-x<1);
    delete request;
    if (CheckResponse(*this,RequestAnswer)!=0)
    {
        return RetrievePersonalInfo();
    }
    int i=RequestAnswer.size();


    while (RequestAnswer[i--]!='}');
    RequestAnswer.erase(i+2,RequestAnswer.size()-i+1);
    RequestAnswer=replacestr(RequestAnswer,"\\/","/");
    RequestAnswer=replacestr(RequestAnswer,"\\t","\t");
    //cout<<RequestAnswer;
    std::stringstream stream(RequestAnswer);
    profile.Clear();
    try
    {
      json::Reader::Read(profile, stream);
    }catch(...)
    {
        RetrievePersonalInfo();
    }
    return 0;
}

int VKObject::GetVkontakteID()
{
    return ((json::Number&)(profile["id"])).Value();
}

string VKObject::GetProfileImagePath()
{
    return ((json::String&)(profile["bp"])).Value();
}

int VKObject::GetPhotosCount()
{
    return ((json::Number&)(profile["ph"]["n"])).Value();
}

string VKObject::GetNPhotoURL(int n)
{
    return ((json::String&)(profile["ph"]["d"][n][2])).Value();
}

string VKObject::GetNMiniPhotoURL(int n)
{
    return ((json::String&)(profile["ph"]["d"][n][1])).Value();
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

int VKWallReader::Retrieve(VKObject& session,int uid,int from, int to)
{
    sess=&session;
    Easy* wall=new Easy;
    WriteFunctionFunctor functor(WriteStringCallback);
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
        return Retrieve(session,uid,from,to);
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

int VKWallReader::GetMessageReceiverID(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][4][0])).Value();
}

string VKWallReader::GetMessageReceiverName(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::String&)(d[n][4][1])).Value();
}


// PM

VKPMReader::VKPMReader(const char* act)
{
    acttype=act;
}

int VKPMReader::Retrieve(VKObject& session,int uid,int from, int to)
{
    sess=&session;
    Easy* wall=new Easy;
    WriteFunctionFunctor functor(WriteStringCallback);
    WriteFunction* cb = new curlpp::options::WriteFunction(functor);
    wall->setOpt(cb);
    wall->setOpt(Url("http://userapi.com/data"));
    wall->setOpt(Post(true));
    wall->setOpt(Header(false));

    std::string fields=session.sid+"&act="+acttype+"&";
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
        return Retrieve(session,uid,from,to);
    }
    std::stringstream stream(RequestAnswer);
    jsonresponse.Clear();
    json::Reader::Read(jsonresponse, stream);
    delete wall;
}

int VKPMReader::MessageCount()
{
    return ((json::Number&)(jsonresponse["n"])).Value();
}

int VKPMReader::GetMessageID(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][0])).Value();
}

int VKPMReader::GetMessageTime(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][1])).Value();
}

string VKPMReader::GetMessageText(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::String&)(d[n][2][0])).Value();
}

int VKPMReader::GetMessageType(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][2][1])).Value();
}

int VKPMReader::GetMessageSenderID(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][3][0])).Value();
}

string VKPMReader::GetMessageSenderName(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::String&)(d[n][3][1])).Value();
}

int VKPMReader::GetMessageReceiverID(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::Number&)(d[n][4][0])).Value();
}

string VKPMReader::GetMessageReceiverName(int n)
{
    json::Array& d=jsonresponse["d"];
    return ((json::String&)(d[n][4][1])).Value();
}

}
