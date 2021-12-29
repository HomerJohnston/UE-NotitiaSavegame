// Copyright (c) 2020 Kyle J Wilcox (HoJo). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNotitia, Log, All);
DECLARE_STATS_GROUP(TEXT("Notitia"), STATGROUP_Notitia, STATCAT_Advanced);

// ================================================================================================
// Logging Settings
#define NOTITIA_LOGGING_ERROR 		1
#define NOTITIA_LOGGING_WARNING 	!(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#define NOTITIA_LOGGING_DEBUG 		UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
#define NOTITIA_LOGGING_VERBOSE 	1
// ================================================================================================


#if NOTITIA_LOGGING_ERROR
	#define NOTITIA_LOG_ERROR(TextArg, ...) UE_LOG(LogNotitia, Error, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIA_LOG_ERROR_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitia, Error, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIA_LOG_ERROR(TextArg, ...)
	#define NOTITIA_LOG_ERROR_IF(Condition, TextArg, ...)
#endif


#if NOTITIA_LOGGING_WARNING
	#define NOTITIA_LOG_WARNING(TextArg, ...) UE_LOG(LogNotitia, Warning, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIA_LOG_WARNING_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitia, Warning, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIA_LOG_WARNING(TextArg, ...)
	#define NOTITIA_LOG_WARNING_IF(Condition, TextArg, ...)
#endif


#if NOTITIA_LOGGING_DEBUG
	#define NOTITIA_LOG_DEBUG(TextArg, ...) UE_LOG(LogNotitia, Display, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIA_LOG_DEBUG_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitia, Display, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIA_LOG_DEBUG(TextArg, ...)
	#define NOTITIA_LOG_DEBUG_IF(Condition, TextArg, ...)
#endif


#if NOTITIA_LOGGING_VERBOSE
	#define NOTITIA_LOG_VERBOSE(TextArg, ...) UE_LOG(LogNotitia, Display, TEXT(TextArg), __VA_ARGS__)
	#define NOTITIA_LOG_VERBOSE_IF(Condition, TextArg, ...) if (Condition) { UE_LOG(LogNotitia, Display, TEXT(TextArg), __VA_ARGS__ ); }
#else
	#define NOTITIA_LOG_VERBOSE(TextArg, ...)
	#define NOTITIA_LOG_VERBOSE_IF(Condition, TextArg, ...)
#endif
