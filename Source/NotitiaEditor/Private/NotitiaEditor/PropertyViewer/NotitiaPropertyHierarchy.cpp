#include "NotitiaEditor/PropertyViewer/NotitiaPropertyHierarchy.h"
#include "NotitiaEditor/PropertyViewer/NotitiaPropertyNodeSortPredicate.h"

FNotitiaPropertyHierarchy::FNotitiaPropertyHierarchy()
{
}

FNotitiaPropertyHierarchy::~FNotitiaPropertyHierarchy()
{
}

void FNotitiaPropertyHierarchy::PopulatePropertyHierarchy(UClass* InSourceClass)
{
	SourceClass = InSourceClass;
	
	RootProperties.Empty();

	TArray<FName> StructContainer;

	for (TFieldIterator<FProperty> PropIt(SourceClass); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		FStructProperty* StructProperty = CastField<FStructProperty>(Property);

		bool bDoAdd = false;
		
		if (StructProperty)
		{
			for (TFieldIterator<FProperty> PropIt2(StructProperty->Struct); PropIt2; ++PropIt2)
			{
				bDoAdd = true;
			}
		}
		else
		{
			bDoAdd = true;
		}

		if (bDoAdd)
		{
			TSharedPtr<FNotitiaPropertyViewerNode> NewNode = MakeShareable(new FNotitiaPropertyViewerNode(InSourceClass, Property, StructContainer));

			RootProperties.Add(NewNode);
		
			AddChildren_NoFilter(NewNode, Property, StructContainer);			
		}
	}

	RootProperties.Sort(FNotitiaPropertyNodeSortPredicate());

	UClass* CurrentOwner = nullptr;

	for (int i = 0; i < RootProperties.Num(); ++i)
	{
		TSharedPtr<FNotitiaPropertyViewerNode>& Node = RootProperties[i];
		
		UClass* PropertyOwnerClass = Node->GetOwnerClass();
		
		if (!CurrentOwner)
		{
			Node->SetDisplayClassHeader();
			CurrentOwner = Node->GetOwnerClass(); 
		}

		if (PropertyOwnerClass!= CurrentOwner)
		{
			Node->SetDisplayClassHeader();
			CurrentOwner = PropertyOwnerClass;
		}

		if (RootProperties.IsValidIndex(i + 1))
		{
			if (RootProperties[i + 1]->GetOwnerClass() != PropertyOwnerClass)
			{
				Node->SetDisplayClassFooter();
			}
		}
	}
}

void FNotitiaPropertyHierarchy::AddChildren_NoFilter(TSharedPtr<FNotitiaPropertyViewerNode>& InOutRootNode, FProperty* ParentProperty, TArray<FName>& StructContainer)
{
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(ParentProperty);

	FInterfaceProperty* InterfaceProperty = CastField<FInterfaceProperty>(ParentProperty);

	FMapProperty* MapProperty = CastField<FMapProperty>(ParentProperty);

	FObjectProperty* ObjectProperty = CastField<FObjectProperty>(ParentProperty);

	FSetProperty* SetProperty = CastField<FSetProperty>(ParentProperty);

	FStructProperty* StructProperty = CastField<FStructProperty>(ParentProperty);

	if (StructProperty)
	{
		StructContainer.Add(ParentProperty->GetFName());
		
		for (TFieldIterator<FProperty> PropIt(StructProperty->Struct); PropIt; ++PropIt)
		{
			FProperty* ChildProperty = *PropIt;

			TSharedPtr<FNotitiaPropertyViewerNode> NewNode = MakeShareable(new FNotitiaPropertyViewerNode(InOutRootNode->SourceClass, ChildProperty, StructContainer));

			InOutRootNode->AddChild(NewNode);
			
			AddChildren_NoFilter(NewNode, ChildProperty, StructContainer);
		}

		InOutRootNode->GetChildrenList().Sort(FNotitiaPropertyNodeSortPredicate());

		StructContainer.Pop();
	}
}
