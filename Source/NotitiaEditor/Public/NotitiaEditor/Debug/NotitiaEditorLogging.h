// Copyright (c) 2020 Kyle J Wilcox (HoJo). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNotitiaEd, Log, All);
DECLARE_STATS_GROUP(TEXT("NotitiaEd"), STATGROUP_NotitiaEd, STATCAT_Advanced);

// ================================================================================================
// Logging Settings
#define NOTITIAED_LOGGING_ERROR 		1
#define NOTITIAED_LOGGING_WARNING 	!(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#define NOTITIAED_LOGGING_DEBUG 		UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
#define NOTITIAED_LOGGING_DEBUG 		UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
#define NOTITIAED_LOGGING_VERBOSE 	1
// ================================================================================================


#if NOTITIAED_LOGGING_ERROR
	#define NOTITIAED_LOG_ERROR(TextArg, ...) UE_LOG(LogNotitiaEd, Error, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIAED_LOG_ERROR_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitiaEd, Error, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIAED_LOG_ERROR(TextArg, ...)
	#define NOTITIAED_LOG_ERROR_IF(Condition, TextArg, ...)
#endif


#if NOTITIAED_LOGGING_WARNING
	#define NOTITIAED_LOG_WARNING(TextArg, ...) UE_LOG(LogNotitiaEd, Warning, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIAED_LOG_WARNING_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitiaEd, Warning, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIAED_LOG_WARNING(TextArg, ...)
	#define NOTITIAED_LOG_WARNING_IF(Condition, TextArg, ...)
#endif


#if NOTITIAED_LOGGING_DEBUG
	#define NOTITIAED_LOG_DEBUG(TextArg, ...) UE_LOG(LogNotitiaEd, Display, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIAED_LOG_DEBUG_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitiaEd, Display, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIAED_LOG_DEBUG(TextArg, ...)
	#define NOTITIAED_LOG_DEBUG_IF(Condition, TextArg, ...)
#endif


#if NOTITIAED_LOGGING_VERBOSE
	#define NOTITIAED_LOG_VERBOSE(TextArg, ...) UE_LOG(LogNotitiaEd, Display, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIAED_LOG_VERBOSE_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitiaEd, Display, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIAED_LOG_VERBOSE(TextArg, ...)
	#define NOTITIAED_LOG_VERBOSE_IF(Condition, TextArg, ...)
#endif
