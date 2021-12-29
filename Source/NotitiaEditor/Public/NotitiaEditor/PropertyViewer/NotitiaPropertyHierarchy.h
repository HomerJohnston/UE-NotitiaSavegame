#pragma  once

#include "CoreMinimal.h"
#include "NotitiaPropertyViewerNode.h"

class FNotitiaPropertyHierarchy
{
protected:
	UClass* SourceClass = nullptr;

	TArray<TSharedPtr<FNotitiaPropertyViewerNode>> RootProperties;

public:
	FNotitiaPropertyHierarchy();

	~FNotitiaPropertyHierarchy();

	void PopulatePropertyHierarchy(UClass* InSourceClass);

	void AddChildren_NoFilter(TSharedPtr<FNotitiaPropertyViewerNode>& InOutRootNode, FProperty* ParentProperty, TArray<FName>& StructContainer);

	TArray<TSharedPtr<FNotitiaPropertyViewerNode>> GetRootProperties() { return RootProperties; };
};
