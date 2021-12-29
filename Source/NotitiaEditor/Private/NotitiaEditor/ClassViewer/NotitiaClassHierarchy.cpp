#include "NotitiaEditor/ClassViewer/NotitiaClassHierarchy.h"

#include "NotitiaEditor/ClassViewer/NotitiaClassViewer.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNodeNameLess.h"
#include "NotitiaEditor/ClassViewer/NotitiaUnloadedBlueprintData.h"
#include "Animation/AnimBlueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/HotReloadInterface.h"
#include "UObject/CoreRedirects.h"

bool FNotitiaClassHierarchy::bPopulateClassHierarchy;

FNotitiaClassHierarchy::FNotitiaClassHierarchy()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	OnFilesLoadedRequestPopulateClassHierarchyDelegateHandle = AssetRegistryModule.Get().OnFilesLoaded().AddStatic(FNotitiaClassHierarchy::RequestPopulateClassHierarchy);

	AssetRegistryModule.Get().OnAssetAdded().AddRaw(this, &FNotitiaClassHierarchy::AddAsset);
	AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &FNotitiaClassHierarchy::RemoveAsset);

	IHotReloadInterface& HotReloadSupport = FModuleManager::LoadModuleChecked<IHotReloadInterface>("HotReload");
	HotReloadSupport.OnHotReload().AddRaw(this, &FNotitiaClassHierarchy::OnHotReload);

	OnBlueprintCompiledRequestPopulateClassHierarchyDelegateHandle = GEditor->OnBlueprintCompiled().AddStatic(FNotitiaClassHierarchy::RequestPopulateClassHierarchy);
	OnClassPackageLoadedOrUnloadedRequestPopulateClassHierarchyDelegateHandle = GEditor->OnClassPackageLoadedOrUnloaded().AddStatic(FNotitiaClassHierarchy::RequestPopulateClassHierarchy);

	FModuleManager::Get().OnModulesChanged().AddStatic(&OnModulesChanged);
}

FNotitiaClassHierarchy::~FNotitiaClassHierarchy()
{
}

void FNotitiaClassHierarchy::PopulateClassHierarchy()
{
	TArray<TSharedPtr<FNotitiaClassViewerNode>> RootLevelClasses;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FAssetData> BlueprintList;

	FARFilter Filter;

	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.ClassNames.Add(UAnimBlueprint::StaticClass()->GetFName());
	Filter.ClassNames.Add(UBlueprintGeneratedClass::StaticClass()->GetFName());

	Filter.bRecursiveClasses = true;
	
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintList);

	TMap<FName, TSharedPtr<FNotitiaClassViewerNode>> ClassPathToNode;

	for (int32 AssetIdx = 0; AssetIdx < BlueprintList.Num(); ++AssetIdx)
	{
		TSharedPtr<FNotitiaClassViewerNode> NewNode;

		LoadUnloadedTagData(NewNode, BlueprintList[AssetIdx]);
		RootLevelClasses.Add(NewNode);

		check(!NewNode->GetChildrenList().Num());

		ClassPathToNode.Add(NewNode->ClassPath, NewNode);

		FindClass(NewNode);
	}

	AddChildren_NoFilter(ObjectClassRoot, ClassPathToNode);

	RootLevelClasses.Add(ObjectClassRoot);

	for (int32 CurrentNodeIdx = 0; CurrentNodeIdx < RootLevelClasses.Num(); ++CurrentNodeIdx)
	{
		if (RootLevelClasses[CurrentNodeIdx]->ParentClassPath != NAME_None)
		{
			FString ParentClassPath = RootLevelClasses[CurrentNodeIdx]->ParentClassPath.ToString();

			TSharedPtr<FNotitiaClassViewerNode> ParentNode;

			TSharedPtr<FNotitiaClassViewerNode>* ParentNodePtr = ClassPathToNode.Find(RootLevelClasses[CurrentNodeIdx]->ParentClassPath);
			
			if (ParentNodePtr)
			{
				ParentNode = *ParentNodePtr;
				ParentNode->AddUniqueChild(RootLevelClasses[CurrentNodeIdx]);
				RootLevelClasses.RemoveAtSwap(CurrentNodeIdx);
				--CurrentNodeIdx;
			}
		}
	}

	SortChildren(ObjectClassRoot);

	SNotitiaClassViewer::PopulateClassViewerDelegate.Broadcast();
}

void FNotitiaClassHierarchy::AddChildren_NoFilter(TSharedPtr<FNotitiaClassViewerNode>& InOutRootNode, TMap<FName, TSharedPtr<FNotitiaClassViewerNode>>& InOutClassPathToNode)
{
	UClass* RootClass = UObject::StaticClass();

	InOutRootNode = MakeShared<FNotitiaClassViewerNode>(RootClass);
	InOutRootNode->SetClass(RootClass);

	InOutClassPathToNode.Add(InOutRootNode->ClassPath, InOutRootNode);

	TMap<UClass*, TSharedPtr<FNotitiaClassViewerNode>> VisitedNodes;
	VisitedNodes.Add(RootClass, InOutRootNode);

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* CurrentClass = *ClassIt;

		if (CurrentClass->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists) || FKismetEditorUtilities::IsClassABlueprintSkeleton(CurrentClass))
		{
			continue;
		}

		TSharedPtr<FNotitiaClassViewerNode>& Entry = VisitedNodes.FindOrAdd(CurrentClass);

		if (Entry.IsValid())
		{
			continue;
		}
		else
		{
			while (CurrentClass->GetSuperClass() != nullptr)
			{
				TSharedPtr<FNotitiaClassViewerNode>& ParentEntry = VisitedNodes.FindOrAdd(CurrentClass->GetSuperClass());

				if (!ParentEntry.IsValid())
				{
					FName ParentClassPath = FName(*CurrentClass->GetSuperClass()->GetPathName());

					TSharedPtr<FNotitiaClassViewerNode>* AlreadyExisting = InOutClassPathToNode.Find(ParentClassPath);

					if (AlreadyExisting)
					{
						ParentEntry = *AlreadyExisting;
					}
					else
					{
						ParentEntry = MakeShared<FNotitiaClassViewerNode>(CurrentClass->GetSuperClass());
						ParentEntry->SetClass(CurrentClass);
						InOutClassPathToNode.Add(ParentEntry->ClassPath, ParentEntry);
					}
				}

				TSharedPtr<FNotitiaClassViewerNode>& MyEntry = VisitedNodes.FindOrAdd(CurrentClass);

				if (!MyEntry.IsValid())
				{
					FName ClassPath = FName(*CurrentClass->GetPathName());

					TSharedPtr<FNotitiaClassViewerNode>* AlreadyExisting = InOutClassPathToNode.Find(ClassPath);

					if (AlreadyExisting)
					{
						MyEntry = *AlreadyExisting;
						ParentEntry->AddUniqueChild(MyEntry);
					}
					else
					{
						MyEntry = MakeShared<FNotitiaClassViewerNode>(CurrentClass);
						InOutClassPathToNode.Add(MyEntry->ClassPath, MyEntry);
						ParentEntry->AddChild(MyEntry);
					}
				}

				CurrentClass = CurrentClass->GetSuperClass();
			}
		}
	}
}

void FNotitiaClassHierarchy::LoadUnloadedTagData(TSharedPtr<FNotitiaClassViewerNode>& InOutClassViewerNode, const FAssetData& InAssetData)
{
	const FString ClassName = InAssetData.AssetName.ToString();

	FString ClassDisplayName = InAssetData.GetTagValueRef<FString>(FBlueprintTags::BlueprintDisplayName);

	if (ClassDisplayName.IsEmpty())
	{
		ClassDisplayName = ClassName;
	}

	InOutClassViewerNode = MakeShared<FNotitiaClassViewerNode>(ClassName, ClassDisplayName);
	InOutClassViewerNode->BlueprintAssetPath = InAssetData.ObjectPath;

	FString ClassObjectPath;
	if (InAssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, ClassObjectPath))
	{
		InOutClassViewerNode->ClassPath = FName(*FPackageName::ExportTextPathToObjectPath(ClassObjectPath));
	}

	FString ParentClassPathString;
	if (InAssetData.GetTagValue(FBlueprintTags::ParentClassPath, ParentClassPathString))
	{
		InOutClassViewerNode->ParentClassPath = FName(*FPackageName::ExportTextPathToObjectPath(ParentClassPathString));
	}

	TSharedPtr<FNotitiaUnloadedBlueprintData> UnloadedBlueprintData = MakeShareable(new FNotitiaUnloadedBlueprintData(InOutClassViewerNode));
	InOutClassViewerNode->UnloadedBlueprintData = UnloadedBlueprintData;

	const bool bNormalBlueprintType = InAssetData.GetTagValueRef<FString>(FBlueprintTags::BlueprintType) == TEXT("BPType_Normal");
	InOutClassViewerNode->UnloadedBlueprintData->SetNormalBlueprintType(bNormalBlueprintType);

	const uint32 ClassFlags = InAssetData.GetTagValueRef<uint32>(FBlueprintTags::ClassFlags);
	InOutClassViewerNode->UnloadedBlueprintData->SetClassFlags(ClassFlags);

	const FString ImplementedInterfaces = InAssetData.GetTagValueRef<FString>(FBlueprintTags::ImplementedInterfaces);
	if (!ImplementedInterfaces.IsEmpty())
	{
		FString FullInterface;
		FString RemainingString;
		FString InterfacePath;
		FString CurrentString = *ImplementedInterfaces;

		while (CurrentString.Split(TEXT(","), &FullInterface, &RemainingString))
		{
			if (!CurrentString.StartsWith(TEXT("Graphs=(")))
			{
				if (FullInterface.Split(TEXT("\""), &CurrentString, &InterfacePath, ESearchCase::CaseSensitive))
				{
					InterfacePath.RemoveFromEnd(TEXT("\"'"));

					FCoreRedirectObjectName ResolvedInterfaceName = FCoreRedirects::GetRedirectedName(ECoreRedirectFlags::Type_Class, FCoreRedirectObjectName(InterfacePath));

					UnloadedBlueprintData->AddImplementedInterface(ResolvedInterfaceName.ObjectName.ToString());
				}
			}

			CurrentString = RemainingString;
		}
	}
}

const TSharedPtr<FNotitiaClassViewerNode> FNotitiaClassHierarchy::GetObjectRootNode() const
{
	check(ObjectClassRoot.IsValid())

	return ObjectClassRoot;
}

TSharedPtr<FNotitiaClassViewerNode> FNotitiaClassHierarchy::FindNodeByGeneratedClassPath(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InGeneratedClassPath)
{
	if (InRootNode->ClassPath == InGeneratedClassPath)
	{
		return InRootNode;
	}

	TSharedPtr<FNotitiaClassViewerNode> ReturnNode;

	for (int32 ChildClassIndex = 0; !ReturnNode.IsValid() && ChildClassIndex < InRootNode->GetChildrenList().Num(); ChildClassIndex++)
	{
		ReturnNode = FindNodeByGeneratedClassPath(InRootNode->GetChildrenList()[ChildClassIndex], InGeneratedClassPath);

		if (ReturnNode.IsValid())
		{
			break;
		}
	}

	return ReturnNode;
}

void FNotitiaClassHierarchy::UpdateClassInNode(FName InGeneratedClassPath, UClass* InNewClass, UBlueprint* InNewBluePrint)
{
	TSharedPtr<FNotitiaClassViewerNode> Node = FindNodeByGeneratedClassPath(ObjectClassRoot, InGeneratedClassPath);

	if (Node.IsValid())
	{
		Node->SetClass(InNewClass);
		Node->Blueprint = InNewBluePrint;
	}
}

void FNotitiaClassHierarchy::RequestPopulateClassHierarchy()
{
	bPopulateClassHierarchy = true;
}

void FNotitiaClassHierarchy::AddAsset(const FAssetData& InAddedAssetData)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	if (!AssetRegistryModule.Get().IsLoadingAssets())
	{
		TArray<FName> AncestorClassNames;

		AssetRegistryModule.Get().GetAncestorClassNames(InAddedAssetData.AssetClass, AncestorClassNames);

		if (AncestorClassNames.Contains(UBlueprintCore::StaticClass()->GetFName()))
		{
			FString ClassObjectPath;
			if (InAddedAssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, ClassObjectPath))
			{
				ClassObjectPath = FPackageName::ExportTextPathToObjectPath(ClassObjectPath);
			}

			if (!FindNodeByGeneratedClassPath(ObjectClassRoot, FName(*ClassObjectPath)).IsValid())
			{
				TSharedPtr<FNotitiaClassViewerNode> NewNode;
				LoadUnloadedTagData(NewNode, InAddedAssetData);

				FindClass(NewNode);

				FString ParentClassPath = NewNode->ParentClassPath.ToString();
				UClass* ParentClass = FindObject<UClass>(nullptr, *ParentClassPath);

				TSharedPtr<FNotitiaClassViewerNode> ParentNode = FindParent(ObjectClassRoot, NewNode->ParentClassPath, ParentClass);

				if (ParentNode.IsValid())
				{
					ParentNode->AddChild(NewNode);

					SortChildren(ObjectClassRoot);

					SNotitiaClassViewer::PopulateClassViewerDelegate.Broadcast();
				}
			}
		}
	}
}

void FNotitiaClassHierarchy::FindClass(TSharedPtr<FNotitiaClassViewerNode> InOutClassNode)
{
	UClass* Class = FindObject<UClass>(nullptr, *InOutClassNode->ClassPath.ToString());

	if (Class)
	{
		InOutClassNode->Blueprint = Cast<UBlueprint>(Class->ClassGeneratedBy);
		InOutClassNode->SetClass(Class);
	}
}

TSharedPtr<FNotitiaClassViewerNode> FNotitiaClassHierarchy::FindParent(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InParentClassName, const UClass* InParentClass)
{
	if (InRootNode->ClassPath == InParentClassName)
	{
		return InRootNode;
	}
	else
	{
		const UClass* ParentClass = InParentClass;

		if (const UClass* RootClass = InRootNode->GetClassWeakPtr().Get())
		{
			if (ParentClass == RootClass)
			{
				return InRootNode;
			}
		}
	}

	TSharedPtr<FNotitiaClassViewerNode> ReturnNode;

	for (int32 ChildClassIndex = 0; !ReturnNode.IsValid() && ChildClassIndex < InRootNode->GetChildrenList().Num(); ChildClassIndex++)
	{
		ReturnNode = FindParent(InRootNode->GetChildrenList()[ChildClassIndex], InParentClassName, InParentClass);

		if (ReturnNode.IsValid())
		{
			break;
		}
	}

	return ReturnNode;
}

void FNotitiaClassHierarchy::SortChildren(TSharedPtr<FNotitiaClassViewerNode>& InRootNode)
{
	TArray<TSharedPtr<FNotitiaClassViewerNode>>& ChildList = InRootNode->GetChildrenList();

	for (int32 ChildIndex = 0; ChildIndex < ChildList.Num(); ChildIndex++)
	{
		SortChildren(ChildList[ChildIndex]);
	}

	ChildList.Sort(FNotitiaClassViewerNodeNameLess());
}

void FNotitiaClassHierarchy::RemoveAsset(const FAssetData& InRemovedAssetData)
{
	FString ClassObjectPath;

	if (InRemovedAssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, ClassObjectPath))
	{
		ClassObjectPath = FPackageName::ExportTextPathToObjectPath(ClassObjectPath);

		if (ClassObjectPath == "None")
		{
			ClassObjectPath = InRemovedAssetData.ObjectPath.ToString() + "_C";
		}
	}

	if (FindAndRemoveNodeByClassPath(ObjectClassRoot, FName(*ClassObjectPath)))
	{
		SNotitiaClassViewer::PopulateClassViewerDelegate.Broadcast();
	}
}

bool FNotitiaClassHierarchy::FindAndRemoveNodeByClassPath(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InClassPath)
{
	bool bReturnValue = false;

	for (int32 ChildClassIndex = 0; ChildClassIndex < InRootNode->GetChildrenList().Num(); ChildClassIndex++)
	{
		if (InRootNode->GetChildrenList()[ChildClassIndex]->ClassPath == InClassPath)
		{
			InRootNode->GetChildrenList().RemoveAt(ChildClassIndex);
			return true;
		}

		bReturnValue = FindAndRemoveNodeByClassPath(InRootNode->GetChildrenList()[ChildClassIndex], InClassPath);

		if (bReturnValue)
		{
			break;
		}
	}
	
	return bReturnValue;
}

void FNotitiaClassHierarchy::OnHotReload(bool bWasTriggeredAutomatically)
{
	// TODO: break everything intentionally 
	RequestPopulateClassHierarchy();
}

void FNotitiaClassHierarchy::OnModulesChanged(FName ModuleThatChanged, EModuleChangeReason ReasonForChange)
{
	RequestPopulateClassHierarchy();
}
