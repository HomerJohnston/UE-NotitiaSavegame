#pragma once

enum class ENotitiaClassViewerNameTypeToDisplay : uint8
{
	/** Display both the display name and class name if they're available and different. */
	Dynamic,

	/** Always use the display name */
	DisplayName,

	/** Always use the class name */
	ClassName,
};