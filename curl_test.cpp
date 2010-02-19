#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>


using namespace cURLpp;
using namespace Options;
using namespace types;
using namespace std;

std::string RequestAnswer;

size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb)
{
  RequestAnswer += ptr;
  int FullSize = size*nmemb;
  return FullSize;
}

int main(int argc, char* argv[])
{
	curlpp::Easy request;
  curlpp::options::Url uHabr(std::string("http://example.com"));
  request.setOpt( Url("http://m.habrahabr.ru") );
  request.setOpt( UserAgent("Curl-Testing Agent") );
  
  WriteFunctionFunctor functor(WriteMemoryCallback);
  WriteFunction* cb = new curlpp::options::WriteFunction(functor);
  request.setOpt(cb);

  request.perform();
  cout<<RequestAnswer<<"\n";
  return 0;
}
