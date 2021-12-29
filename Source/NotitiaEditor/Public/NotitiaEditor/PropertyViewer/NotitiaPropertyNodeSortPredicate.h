#pragma once

#include "CoreMinimal.h"

class FNotitiaPropertyViewerNode;

struct FNotitiaPropertyNodeSortPredicate
{
	bool operator()(TSharedPtr<FNotitiaPropertyViewerNode> A, TSharedPtr<FNotitiaPropertyViewerNode> B) const;
	
	FNotitiaPropertyNodeSortPredicate() { };
};
