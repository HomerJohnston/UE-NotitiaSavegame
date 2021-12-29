#include "NotitiaEditor/PropertyViewer/NotitiaPropertyNodeSortPredicate.h"

#include "NotitiaEditor/PropertyViewer/NotitiaPropertyViewerNode.h"

bool FNotitiaPropertyNodeSortPredicate::operator()(TSharedPtr<FNotitiaPropertyViewerNode> A, TSharedPtr<FNotitiaPropertyViewerNode> B) const
{
	check(A.IsValid());
	check(B.IsValid());

	// The display name only matters when NameTypeToDisplay == DisplayName. For NameTypeToDisplay == Dynamic,
	// the class name is displayed first with the display name in parentheses, but only if it differs from the display name.
	const FString& NameA = *A->GetPropertyDisplayName();
	const FString& NameB = *B->GetPropertyDisplayName();

	const bool NotFromSuperClassA = A->PropertyNotFromSuperClass();
	const bool NotFromSuperClassB = B->PropertyNotFromSuperClass();

	if (NotFromSuperClassA && NotFromSuperClassB)
	{
		return NameA.Compare(NameB, ESearchCase::IgnoreCase) < 0;
	}

	if (NotFromSuperClassA != NotFromSuperClassB)
	{
		return NotFromSuperClassA;
	}

	UClass* ClassA = A->GetOwnerClass();
	UClass* ClassB = B->GetOwnerClass();

	if (ClassA == ClassB)
	{
		return NameA.Compare(NameB, ESearchCase::IgnoreCase) < 0;
	}
	
	if (ClassA->IsChildOf(ClassB) || ClassB->IsChildOf(ClassA))
	{
		return ClassA->IsChildOf(ClassB);
	}
	
	return ClassA->GetName().Compare(ClassB->GetName(), ESearchCase::IgnoreCase) < 0;
}
