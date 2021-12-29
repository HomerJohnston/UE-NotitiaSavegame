#pragma once

class INotitiaUnloadedBlueprintData
{
public:

	/**
	 * Used to safely check whether the passed in flag is set.
	 *
	 * @param	InFlagsToCheck		Class flag to check for
	 *
	 * @return	true if the passed in flag is set, false otherwise
	 *			(including no flag passed in, unless the FlagsToCheck is CLASS_AllFlags)
	 */
	virtual bool HasAnyClassFlags( uint32 InFlagsToCheck ) const = 0;

	/**
	 * Used to safely check whether all of the passed in flags are set.
	 *
	 * @param InFlagsToCheck	Class flags to check for
	 * @return true if all of the passed in flags are set (including no flags passed in), false otherwise
	 */
	virtual bool HasAllClassFlags( uint32 InFlagsToCheck ) const = 0;

	/** 
	 * Sets the flags for this class.
	 *
	 * @param InFlags		The flags to be set to.
	 */
	virtual void SetClassFlags(uint32 InFlags) = 0;

	/** 
	 * This will return whether or not this class implements the passed in class / interface 
	 *
	 * @param InInterface - the interface to check and see if this class implements it
	 **/
	virtual bool ImplementsInterface(const UClass* InInterface) const = 0;

	/**
	 * Checks whether or not the class is a child-of the passed in class.
	 *
	 * @param InClass		The class to check against.
	 *
	 * @return				true if it is a child-of the passed in class.
	 */
	virtual bool IsChildOf(const UClass* InClass) const = 0;

	/** 
	 * Checks whether or not the class has an Is-A relationship with the passed in class.
	 *
	 * @param InClass		The class to check against.
	 *
	 * @return				true if it has an Is-A relationship to the passed in class.
	 */
	virtual bool IsA(const UClass* InClass) const = 0;

	/**
	 * Attempts to get the ClassWithin property for this class.
	 *
	 * @return ClassWithin of the child most Native class in the hierarchy.
	 */
	virtual const UClass* GetClassWithin() const = 0;

	/**
	 * Attempts to get the child-most Native class in the hierarchy.
	 *
	 * @return The child-most Native class in the hierarchy.
	 */
	virtual const UClass* GetNativeParent() const = 0;

	/** 
	 * Set whether or not this blueprint is a normal blueprint.
	 */
	virtual void SetNormalBlueprintType(bool bInNormalBPType) = 0;

	/** 
	 * Get whether or not this blueprint is a normal blueprint. 
	 */
	virtual bool IsNormalBlueprintType() const = 0;

	/**
	 * Get the generated class name of this blueprint.
	 */
	virtual TSharedPtr<FString> GetClassName() const = 0;

	/**
	 * Get the class path of this blueprint.
	 */
	virtual FName GetClassPath() const = 0;
};
