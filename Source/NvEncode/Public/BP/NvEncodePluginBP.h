#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Tickable.h"

#include "NvEncodePlugin.h"
#include "NvEncodeUtils.h"

#include "NvEncodePluginBP.generated.h"

using namespace NvEncode;

UENUM(BlueprintType)
enum class ENvEncodeErrorState : uint8 {
	Ok,
	Fail
};



/*UCLASS(BlueprintType)
class OPENCV_API UOpenCVTask : public UObject, public FTickableGameObject {
	GENERATED_UCLASS_BODY()
public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FProcessDelegate, UTexture2D*, result);



	bool IsValid() const { return _workload != nullptr; }

	/**
	* Test validity of OpenCVTask
	*
	* @param	Test			The object to test
	* @return	Return true if the object is usable
	*/
	/*UFUNCTION(BlueprintPure, Meta = (CompactNodeTitle = "IsValid"))
		static bool IsValid(const UOpenCVTask* Test) { return IsValidT(Test); }


private:
	bool _IsReady() const {
		if(!IsValid()) LogMessageOnScreen(L"Using invalid StreamSource");
		else if(!_workload->CheckParameters()) LogMessageOnScreen(L"OpenCV task has invalid parameters");
		else if(!asyncEngine.IsAvailable()) LogMessageOnScreen(L"Async operation in progress on this StreamSource");
		else return true;
		return false;
	}

	FProcessDelegate _delegate;
	std::shared_ptr<Workload> _workload;
};*/

/*UCLASS(BlueprintType)
class OPENCV_API UOpenCVUtility : public UBlueprintFunctionLibrary {
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "OpenCV", meta = (WorldContext = WorldContextObject))
		static void GetOpenCVManager(UObject* WorldContextObject, UOpenCVManagerBP*& manager) {
		if(GEngine == nullptr || GEngine->GetWorldFromContextObject(WorldContextObject) == nullptr) return;
		auto* world = GEngine->GetWorldFromContextObject(WorldContextObject);
		if(world->GetAuthGameMode() == nullptr) return;
		manager = world->GetAuthGameMode()->FindComponentByClass<UOpenCVManagerBP>();
	}

};*/

