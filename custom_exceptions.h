#include <exception>
#include <sstream>
#include <string>

using namespace std;

#define DefineException(name)\
class name: public _CustomException {\
public:\
   name(const string &file, int line, const string &err_msg = #name):\
      _CustomException(file, line, err_msg) {}\
};

class _CustomException : public exception {
protected:
   string _err_msg;

public:
   _CustomException(
      const string &file, int line, 
      const string &err_msg = "_CustomException") {
         stringstream ss;
         ss << err_msg << ": " << file << ", " << line;   
         _err_msg = ss.str();
      }

   virtual const char* what() const throw() {
      return _err_msg.c_str();
   }
};

DefineException(NullPointerException)
#define NullPointerError() NullPointerException(__FILE__, __LINE__)

DefineException(DetachedNodeException)
#define DetachedNodeError() DetachedNodeException(__FILE__, __LINE__)

DefineException(NotChildException)
#define NotChildError() NotChildException(__FILE__, __LINE__)

DefineException(PointerToSelfException)
#define PointerToSelfError() PointerToSelfException(__FILE__, __LINE__)

DefineException(MustHaveExactlyOneChildException)
#define MustHaveExactlyOneChildError()\
    MustHaveExactlyOneChildException(__FILE__, __LINE__)

DefineException(NotYetImplementedException)
#define NotYetImplementedError()\
   NotYetImplementedException(__FILE__, __LINE__)

DefineException(NotLeafException)
#define NotLeafError() NotLeafException(__FILE__, __LINE__)

DefineException(LessThanOneException)
#define LessThanOneError() LessThanOneException(__FILE__, __LINE__)
