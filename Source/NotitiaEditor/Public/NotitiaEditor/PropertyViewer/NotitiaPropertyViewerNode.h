#pragma once

#include "CoreMinimal.h"

class FNotitiaPropertyViewerNode : public TSharedFromThis<FNotitiaPropertyViewerNode>
{
public:
	FNotitiaPropertyViewerNode(UClass* InSourceClass, FProperty* InSourceProperty, TArray<FName> InStructPath);

public:
	UClass* SourceClass;

	FProperty* SourceProperty;

protected:
	TWeakPtr<FNotitiaPropertyViewerNode> ParentNode;

	TArray<FName> ParentPath;

	bool bIsStruct;
	
	TArray<TSharedPtr<FNotitiaPropertyViewerNode>> ChildrenList;

	bool bIsDirty;

	bool bDisplayClassHeader;

	bool bDisplayClassFooter;

public:
	TArray<TSharedPtr<FNotitiaPropertyViewerNode>>& GetChildrenList();

	FString GetPropertyDisplayName();
	
	FString GetPropertyType();

	UClass* GetOwnerClass();
	
	bool PropertyNotFromSuperClass();
	
	void AddChild(TSharedPtr<FNotitiaPropertyViewerNode> Child);

	void OnCheckStateChanged(ECheckBoxState NewState);

	void SetPropertyChecked(bool bRecurseIntoChildren = false, bool bRecurseIntoParents = false);
	
	void SetPropertyUnchecked(bool bRecurseIntoChildren = false, bool bRecurseIntoParents = false);
	
	ECheckBoxState GetPropertyVisibleCheckState(bool bCheckChildren = true);

	bool AllChildrenChecked(TWeakPtr<FNotitiaPropertyViewerNode> Node);

	void SetAllChildrenChecked();

	void SetAllChildrenUnchecked();
	
	bool CleanDirty();

	void MarkDirty();
	
	void SetDisplayClassHeader();

	bool GetDisplayClassHeader();
	
	void SetDisplayClassFooter();

	bool GetDisplayClassFooter();
};
