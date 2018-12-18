using UnrealBuildTool;

public class Quady : ModuleRules
{
	public Quady(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

	    if (Target.Version.MinorVersion <= 19)
	    {
	        PublicIncludePaths.AddRange(
	            new string[]
	            {
	            });

	        PrivateIncludePaths.AddRange(
	            new string[]
	            {
	            });
	    }

	    PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			    "CoreUObject",
			    "Engine",
            });
	}
}
