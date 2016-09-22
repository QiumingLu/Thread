#include "mythread/singleton.h"
#include "mythread/thread.h"

#include <string>


namespace mythread {

class SingletonTest {
 public:
  SingletonTest() { }
  const std::string& name() const { return name_; }
  void setName(const std::string& n) { name_ = n; }

 private:
  std::string name_;
};

void ThreadFunc(void*) {
  Singleton<SingletonTest>::Instance()->setName("change singleton name");
}

void Test() {
  Singleton<SingletonTest>::Instance()->setName("singleton test");
  Thread t(&ThreadFunc, NULL);
  t.Start();
  t.Join();
  printf("name=%s\n", Singleton<SingletonTest>::Instance()->name().c_str());
}

}  // namespace voyager

int main() {
  mythread::Test();
  return 0;
}