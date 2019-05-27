#include <thread>
#include <atomic>
#include <Engine.h>

extern void NvEncode::LogMessage(const char* msg);

template <typename T>
bool IsValidT(const T* Test) {
	return ::IsValid(Test) && Test->IsValid();
}

namespace NvEncode {

inline char* allocCString(const FString& str) {
	const auto cast = StringCast<char>(*str);
	char* res = (char*)FMemory::SystemMalloc(cast.Length() + 1);
	FMemory::Memcpy(res, cast.Get(), cast.Length());
	res[cast.Length()] = 0;
	return res;
}

inline void freeCString(const char* cstr) {
	FMemory::SystemFree(const_cast<char*>(cstr));
}

template <typename T1, typename... Ts>
inline bool _valid(T1* obj, Ts*... pack) {
	return IsValid(obj) && _valid(pack...);
}

template <typename Last>
inline bool _valid(Last* obj) {
	return IsValid(obj);
}

template <typename... ObjectsT>
inline bool checkValid(ObjectsT*... objects) {
	if(_valid(objects...)) {
		return true;
	} else {
		OpenCVLog("Trying to use uninitialized object");
		return false;
	}
}

}


