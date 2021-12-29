#include "NotitiaEditor/PropertyViewer/NotitiaPropertyViewerNode.h"

#include "Notitia/Core/NotitiaDeveloperSettings.h"
#include "NotitiaEditor/Debug/NotitiaEditorLogging.h"

FNotitiaPropertyViewerNode::FNotitiaPropertyViewerNode(UClass* InSourceClass, FProperty* InSourceProperty, TArray<FName> InStructPath)
{
	SourceClass = InSourceClass;

	SourceProperty = InSourceProperty;

	bIsStruct = !!CastField<FStructProperty>(InSourceProperty);
	
	ParentPath = InStructPath;

	bIsDirty = true;

	bDisplayClassHeader = false;
	bDisplayClassFooter = false;

	if (InSourceProperty)
	{
		FString PropertyName = InSourceProperty->GetNameCPP();
	
		UClass* OwnerClass = InSourceProperty->GetOwnerClass();
		UStruct* OwnerStruct = InSourceProperty->GetOwnerStruct();
		UField* OwnerField = InSourceProperty->GetOwnerUField();
		
		FString OwnerClassString = OwnerClass ? OwnerClass->GetName() : "None";
		FString OwnerStructString = OwnerClass ? OwnerStruct->GetName() : "None";
		FString OwnerFieldString = OwnerClass ? OwnerField->GetName() : "None";
	}
}

TArray<TSharedPtr<FNotitiaPropertyViewerNode>>& FNotitiaPropertyViewerNode::GetChildrenList()
{
	return ChildrenList;
}

FString FNotitiaPropertyViewerNode::GetPropertyDisplayName()
{
	if (!SourceProperty)
	{
		return "-------------";
	}
	
	FString Name = SourceProperty->GetName();

	/*
	if (!PropertyNotFromSuperClass())
	{
		UClass* OwnerClass = GetOwnerClass();

		if (OwnerClass)
		{
			Name += " (" + FString(OwnerClass->GetPrefixCPP()) + GetOwnerClass()->GetName() + ")";
		}
	}*/
	
	return Name;
}

FString FNotitiaPropertyViewerNode::GetPropertyType()
{
	if (!SourceProperty)
	{
		return "===========";
	}
	
	return SourceProperty->GetCPPType();
}

UClass* FNotitiaPropertyViewerNode::GetOwnerClass()
{
	if (!SourceProperty)
	{
		return nullptr;
	}
	
	UClass* OwnerClass = SourceProperty->GetOwnerClass();

	if (!OwnerClass && ParentNode.IsValid())
	{
		OwnerClass = ParentNode.Pin()->GetOwnerClass();
	}

	return OwnerClass;
}

bool FNotitiaPropertyViewerNode::PropertyNotFromSuperClass()
{
	if (!SourceProperty)
	{
		return false;
	}
	
	UClass* OwnerClass = SourceProperty->GetOwnerClass();

	if (OwnerClass)
	{
		return (OwnerClass == SourceClass);
	}
	else if (ParentNode.IsValid())
	{
		return ParentNode.Pin()->PropertyNotFromSuperClass();
	}

	UE_LOG(LogTemp, Warning, TEXT("PropertyIsFromCurrentClass fell out"));
	return false;
}

void FNotitiaPropertyViewerNode::AddChild(TSharedPtr<FNotitiaPropertyViewerNode> Child)
{
	if (!SourceProperty)
	{
		// TODO log error
		return;
	}
	
	check(Child.IsValid());

	Child->ParentNode = AsShared();
	
	ChildrenList.Add(Child);
}

void FNotitiaPropertyViewerNode::OnCheckStateChanged(ECheckBoxState NewState)
{
	switch (NewState)
	{
		case ECheckBoxState::Checked:
		{
			SetPropertyChecked(true, true);
			break;
		}
		case ECheckBoxState::Unchecked:
		{
			SetPropertyUnchecked(true, true);
			break;
		}
		default:
		{
			// TODO Error handling for impossible input
			break;
		}
	}

	bIsDirty = true;

	UNotitiaDeveloperSettings* DeveloperSettings = GetMutableDefault<UNotitiaDeveloperSettings>();

	DeveloperSettings->SaveConfig();
	DeveloperSettings->UpdateDefaultConfigFile();
}

void FNotitiaPropertyViewerNode::SetPropertyChecked(bool bRecurseIntoChildren, bool bRecurseIntoParents)
{
	if (!SourceProperty)
	{
		// TODO log error
		return;
	}

	UNotitiaDeveloperSettings* DeveloperSettings = GetMutableDefault<UNotitiaDeveloperSettings>();
	
	UClass* OwningClass = SourceProperty->GetOwnerClass();

	if (!OwningClass)
	{
		OwningClass = SourceClass;
	}
	
	TArray<FName> PropertyPath = ParentPath;
	PropertyPath.Add(SourceProperty->GetFName());

	if (bRecurseIntoChildren)
	{
		for (int i = 0; i < ChildrenList.Num(); ++i)
		{
			ChildrenList[i]->SetPropertyUnchecked(true, false);
		}
	}
	
	DeveloperSettings->AddProperty(OwningClass, PropertyPath);

	if (bRecurseIntoParents)
	{
		TSharedPtr<FNotitiaPropertyViewerNode> ParentNodePtr = ParentNode.Pin();
	
		if (ParentNode.IsValid() && AllChildrenChecked(ParentNode))
		{
			ParentNode.Pin()->SetPropertyChecked(false, true);
			ParentNode.Pin()->SetAllChildrenUnchecked();
		}
	}
	
	MarkDirty();
}

void FNotitiaPropertyViewerNode::SetPropertyUnchecked(bool bRecurseIntoChildren, bool bRecurseIntoParents)
{
	if (!SourceProperty)
	{
		// TODO log error
		return;
	}
	
	UNotitiaDeveloperSettings* DeveloperSettings = GetMutableDefault<UNotitiaDeveloperSettings>();

	UClass* OwningClass = SourceProperty->GetOwnerClass();
		
	if (!OwningClass)
	{
		OwningClass = SourceClass;
	}
	
	TArray<FName> PropertyPath = ParentPath;
	PropertyPath.Add(SourceProperty->GetFName());

	if (bRecurseIntoChildren)
	{
		for (int i = 0; i < ChildrenList.Num(); ++i)
		{
			ChildrenList[i]->SetPropertyUnchecked(true, false);
		}
	}

	if (bRecurseIntoParents)
	{
		if (ParentNode.IsValid() && ParentNode.Pin()->GetPropertyVisibleCheckState() == ECheckBoxState::Checked)// DeveloperSettings->GetPropertyState(OwningClass, ParentPath) == ECheckBoxState::Checked)
		{
			// This thing is only checked because its parent is checked. In this case all of its siblings must be the same. Break the parent up.
			ParentNode.Pin()->SetPropertyUnchecked(false, true);
			ParentNode.Pin()->SetAllChildrenChecked();
		}
	}

	DeveloperSettings->RemoveProperty(OwningClass, PropertyPath);
	
	MarkDirty();
}

ECheckBoxState FNotitiaPropertyViewerNode::GetPropertyVisibleCheckState(bool bCheckChildren)
{
	if (!SourceProperty)
	{
		// TODO log error
		return ECheckBoxState::Unchecked;
	}

	const UNotitiaDeveloperSettings* DeveloperSettings = GetDefault<UNotitiaDeveloperSettings>();
	UClass* OwningClass = GetOwnerClass();// SourceProperty->GetOwnerClass();
	
	if (!OwningClass)
	{
		OwningClass = SourceClass;
	}
	
	TArray<FName> PropertyPath = ParentPath;
	PropertyPath.Add(SourceProperty->GetFName());
	
	if (ParentNode.IsValid())
	{
		if (ParentNode.Pin()->GetPropertyVisibleCheckState(false) == ECheckBoxState::Checked)
		{
			return ECheckBoxState::Checked;
		}
	}
	
	// Structs are CHECKED only if they are actually CHECKED. Otherwise, they are undetermined if any of their children are checked, or unchecked otherwise
	if (bIsStruct)
	{
		if (DeveloperSettings->GetPropertyState(OwningClass, PropertyPath) == ECheckBoxState::Checked)
		{
			return ECheckBoxState::Checked;
		}
		else if (bCheckChildren) 
		{
			for (int i = 0; i < ChildrenList.Num(); ++i)
			{
				ECheckBoxState ChildState = ChildrenList[i]->GetPropertyVisibleCheckState();
				if (ChildState != ECheckBoxState::Unchecked)
				{
					return  ECheckBoxState::Undetermined;
				}
			}
		}

		return ECheckBoxState::Unchecked;
	}
	else
	{
		return DeveloperSettings->GetPropertyState(OwningClass, PropertyPath);
	}
	
	/*
	if (bIsStruct)
	{
		bool bAnyStateChecked = false;
		bool bAllStatesChecked = true;
		
		for (int i = 0; i < ChildrenList.Num(); ++i)
		{
			ECheckBoxState ChildState = ChildrenList[i]->GetPropertySelectedState();
			if (ChildState == ECheckBoxState::Checked)
			{
				bAnyStateChecked = true;
			}
			else if (ChildState == ECheckBoxState::Undetermined)
			{
				bAnyStateChecked = true;
				bAllStatesChecked = false;
			}
			else
			{
				bAllStatesChecked = false;
			}
		}

		if (bAllStatesChecked)
		{
			return ECheckBoxState::Checked;
		}
		else if (bAnyStateChecked)
		{
			return ECheckBoxState::Undetermined;
		}
		else
		{
			return ECheckBoxState::Unchecked;
		}
	}
	else
	{
		const UNotitiaDeveloperSettings* DeveloperSettings = GetDefault<UNotitiaDeveloperSettings>();

		UClass* OwningClass = GetOwnerClass();// SourceProperty->GetOwnerClass();

		if (!OwningClass)
		{
			OwningClass = SourceClass;
		}
		
		TArray<FName> PropertyPath = ParentPath;
		PropertyPath.Add(SourceProperty->GetFName());
	
		return DeveloperSettings->GetPropertyState(OwningClass, PropertyPath);	
	}
	*/
}

bool FNotitiaPropertyViewerNode::AllChildrenChecked(TWeakPtr<FNotitiaPropertyViewerNode> Node)
{
	TSharedPtr<FNotitiaPropertyViewerNode> NodePtr = Node.Pin();
	
	for (int i = 0; i < NodePtr->ChildrenList.Num(); ++i)
	{
		ECheckBoxState ChildState = NodePtr->ChildrenList[i]->GetPropertyVisibleCheckState();
		if (ChildState != ECheckBoxState::Checked)
		{
			return false;
		}
	}

	return true;
}

void FNotitiaPropertyViewerNode::SetAllChildrenChecked()
{
	for (int i = 0; i < ChildrenList.Num(); ++i)
	{
		ChildrenList[i]->SetPropertyChecked();
	}
}

void FNotitiaPropertyViewerNode::SetAllChildrenUnchecked()
{
	for (int i = 0; i < ChildrenList.Num(); ++i)
	{
		ChildrenList[i]->SetPropertyUnchecked(true, false);
	}
}

bool FNotitiaPropertyViewerNode::CleanDirty()
{
	bool bWasDirty = bIsDirty;

	bIsDirty = false;

	return bWasDirty;
}

void FNotitiaPropertyViewerNode::MarkDirty()
{
	if (bIsDirty)
	{
		return;
	}
	
	bIsDirty = true;

	if (ParentNode.IsValid())
	{
		ParentNode.Pin()->MarkDirty();
	}

	for (TSharedPtr<FNotitiaPropertyViewerNode> Child : ChildrenList)
	{
		Child->MarkDirty();
	}
}

void FNotitiaPropertyViewerNode::SetDisplayClassHeader()
{
	bDisplayClassHeader = true;
}

bool FNotitiaPropertyViewerNode::GetDisplayClassHeader()
{
	return bDisplayClassHeader;
}

void FNotitiaPropertyViewerNode::SetDisplayClassFooter()
{
	bDisplayClassFooter = true;
}

bool FNotitiaPropertyViewerNode::GetDisplayClassFooter()
{
	return bDisplayClassFooter;
}
