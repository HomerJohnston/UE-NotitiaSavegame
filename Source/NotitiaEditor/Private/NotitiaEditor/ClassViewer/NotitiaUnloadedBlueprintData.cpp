#include "NotitiaEditor/ClassViewer/NotitiaUnloadedBlueprintData.h"

// Copyright Epic Games, Inc. All Rights Reserved.

#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"
#include "Engine/BlueprintGeneratedClass.h"


FNotitiaUnloadedBlueprintData::FNotitiaUnloadedBlueprintData(TWeakPtr<FNotitiaClassViewerNode> InClassViewerNode)
	: ClassViewerNode(InClassViewerNode)
{
}

bool FNotitiaUnloadedBlueprintData::HasAnyClassFlags( uint32 InFlagsToCheck ) const
{
	return (ClassFlags & InFlagsToCheck) != 0;
}

bool FNotitiaUnloadedBlueprintData::HasAllClassFlags( uint32 InFlagsToCheck ) const
{
	return ((ClassFlags & InFlagsToCheck) == InFlagsToCheck);
}

void FNotitiaUnloadedBlueprintData::SetClassFlags(uint32 InFlags)
{
	ClassFlags = InFlags;
}

bool FNotitiaUnloadedBlueprintData::IsChildOf(const UClass* InClass) const
{
	TSharedPtr<FNotitiaClassViewerNode> CurrentNode = ClassViewerNode.Pin()->ParentNode.Pin();

	// Keep going through parents till you find an invalid.
	while (CurrentNode.IsValid())
	{
		if (CurrentNode->GetClassWeakPtr() == InClass)
		{
			return true;
		}
		CurrentNode = CurrentNode->ParentNode.Pin();
	}

	return false;
}

bool FNotitiaUnloadedBlueprintData::ImplementsInterface(const UClass* InInterface) const
{
	// Does this blueprint implement the interface directly?
	for (const FString& DirectlyImplementedInterface : ImplementedInterfaces)
	{
		if (DirectlyImplementedInterface == InInterface->GetName())
		{
			return true;
		}
	}

	// If not, does a parent class implement the interface?
	TSharedPtr<FNotitiaClassViewerNode> CurrentNode = ClassViewerNode.Pin()->ParentNode.Pin();
	while (CurrentNode.IsValid())
	{
		if (CurrentNode->GetClassWeakPtr().IsValid() && CurrentNode->GetClassWeakPtr()->ImplementsInterface(InInterface))
		{
			return true;
		}
		else if (CurrentNode->UnloadedBlueprintData.IsValid() && CurrentNode->UnloadedBlueprintData->ImplementsInterface(InInterface))
		{
			return true;
		}
		CurrentNode = CurrentNode->ParentNode.Pin();
	}

	return false;
}

bool FNotitiaUnloadedBlueprintData::IsA(const UClass* InClass) const
{
	// Unloaded blueprints will always return true for IsA(UBlueprintGeneratedClass::StaticClass). With that in mind, even though we do not have the exact class, we can use that knowledge as a basis for a check.
	return ((UObject*)UBlueprintGeneratedClass::StaticClass())->IsA(InClass);
}

const UClass* FNotitiaUnloadedBlueprintData::GetClassWithin() const
{
	TSharedPtr<FNotitiaClassViewerNode> CurrentNode = ClassViewerNode.Pin()->ParentNode.Pin();

	while (CurrentNode.IsValid())
	{
		// The class field will be invalid for unloaded classes.
		// However, it should be valid once we've hit a loaded class or a Native class.
		// Assuming BP cannot change ClassWithin data, this should be safe.
		if (CurrentNode->GetClassWeakPtr().IsValid())
		{
			return CurrentNode->GetClassWeakPtr()->ClassWithin;
		}

		CurrentNode = CurrentNode->ParentNode.Pin();
	}

	return nullptr;
}

const UClass* FNotitiaUnloadedBlueprintData::GetNativeParent() const
{
	TSharedPtr<FNotitiaClassViewerNode> CurrentNode = ClassViewerNode.Pin()->ParentNode.Pin();

	while (CurrentNode.IsValid())
	{
		// The class field will be invalid for unloaded classes.
		// However, it should be valid once we've hit a loaded class or a Native class.
		// Assuming BP cannot change ClassWithin data, this should be safe.
		if (CurrentNode->GetClassWeakPtr().IsValid() && CurrentNode->GetClassWeakPtr()->HasAnyClassFlags(CLASS_Native))
		{
			return CurrentNode->GetClassWeakPtr().Get();
		}

		CurrentNode = CurrentNode->ParentNode.Pin();
	}

	return nullptr;
}

TSharedPtr<FString> FNotitiaUnloadedBlueprintData::GetClassName() const
{
	if (ClassViewerNode.IsValid())
	{
		return ClassViewerNode.Pin()->GetClassName();
	}

	return TSharedPtr<FString>();
}

FName FNotitiaUnloadedBlueprintData::GetClassPath() const
{
	if (ClassViewerNode.IsValid())
	{
		return ClassViewerNode.Pin()->ClassPath;
	}

	return FName();
}

const TWeakPtr<FNotitiaClassViewerNode>& FNotitiaUnloadedBlueprintData::GetClassViewerNode() const
{
	return ClassViewerNode;
}

void FNotitiaUnloadedBlueprintData::AddImplementedInterface(const FString& InterfaceName)
{
	ImplementedInterfaces.Add(InterfaceName);
}
