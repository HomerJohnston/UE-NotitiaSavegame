#pragma once

#include "CoreMinimal.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNameTypeToDisplay.h"

class FNotitiaClassViewerNode;

struct FNotitiaClassViewerNodeNameLess
{
	ENotitiaClassViewerNameTypeToDisplay NameTypeToDisplay;
	
	bool operator()(TSharedPtr<FNotitiaClassViewerNode> A, TSharedPtr<FNotitiaClassViewerNode> B) const;
	
	FNotitiaClassViewerNodeNameLess(ENotitiaClassViewerNameTypeToDisplay NameTypeToDisplay = ENotitiaClassViewerNameTypeToDisplay::ClassName) : NameTypeToDisplay(NameTypeToDisplay) { };
};
