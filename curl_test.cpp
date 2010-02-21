#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "vklib/vklib.h"
using namespace std;

int main(int argc, char* argv[])
{
  vklib::VKObject obj;
  obj.Login("rader@nm.ru","xtnr5mvY");
  obj.RetrievePersonalInfo();
  cout<<obj.GetProfileImagePath()<<"\n";
  cout<<obj.GetProfileImageSize()<<"\n";
  return 0;
}
