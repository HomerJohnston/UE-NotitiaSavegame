#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNodeNameLess.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNameTypeToDisplay.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"

bool FNotitiaClassViewerNodeNameLess::operator()(TSharedPtr<FNotitiaClassViewerNode> A, TSharedPtr<FNotitiaClassViewerNode> B) const
{ 
	check(A.IsValid());
	check(B.IsValid());

	// The display name only matters when NameTypeToDisplay == DisplayName. For NameTypeToDisplay == Dynamic,
	// the class name is displayed first with the display name in parentheses, but only if it differs from the display name.
	bool bUseDisplayName = NameTypeToDisplay == ENotitiaClassViewerNameTypeToDisplay::DisplayName;
	const FString& NameA = *A->GetClassName(bUseDisplayName).Get();
	const FString& NameB = *B->GetClassName(bUseDisplayName).Get();
	return NameA.Compare(NameB, ESearchCase::IgnoreCase) < 0;
}
