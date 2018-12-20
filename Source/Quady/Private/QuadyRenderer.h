#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "Engine/EngineTypes.h"
#include "Templates/RefCounting.h"
#include "Containers/ArrayView.h"
#include "ShaderParameters.h"
#include "RenderResource.h"
#include "UniformBuffer.h"
#include "VertexFactory.h"
#include "MaterialShared.h"
#include "QuadyMeshProxy.h"
#include "RendererInterface.h"
#include "MeshBatch.h"
#include "SceneManagement.h"
#include "Engine/MapBuildDataRegistry.h"
#include "QuadyMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "StaticMeshResources.h"

#define QUADY_LOD_LEVELS 8

/** The uniform shader parameters for a Quady draw call. */
BEGIN_UNIFORM_BUFFER_STRUCT(FQuadyUniformShaderParameters, QUADY_API)
    /** vertex shader parameters */
    UNIFORM_MEMBER(FVector4, SubsectionSizeVertsLayerUVPan)
    UNIFORM_MEMBER(FVector4, SubsectionOffsetParams)
    UNIFORM_MEMBER(FMatrix, LocalToWorldNoScaling)
END_UNIFORM_BUFFER_STRUCT(FLandscapeUniformShaderParameters)

/* Data needed for the quady vertex factory to set the render state for an individual batch element */
struct FQuadyBatchElementParameters
{
    const TUniformBuffer<FQuadyUniformShaderParameters>* QuadyUniformShaderParametersResource;
    const FMatrix* LocalToWorldNoScalingPtr;

    // LOD calculation-related params
    const FLandscapeComponentSceneProxy* SceneProxy;
    int32 SubX;
    int32 SubY;
    int32 CurrentLOD;
};

class FLandscapeElementParameterArray
    : public FOneFrameResource
{
public:
    TArray<FQuadyBatchElementParameters, SceneRenderingAllocator> ElementParameters;
};

/** Pixel shader parameters for use with FQuadyVertexFactory */
class FQuadyVertexFactoryPixelShaderParameters 
    : public FVertexFactoryShaderParameters
{
public:
    /**
    * Bind shader constants by name
    * @param	ParameterMap - mapping of named shader constants to indices
    */
    virtual void Bind(const FShaderParameterMap& ParameterMap) override;

    /**
    * Serialize shader params to an archive
    * @param	Ar - archive to serialize to
    */
    virtual void Serialize(FArchive& Ar) override;

    /**
    * Set any shader data specific to this vertex factory
    */
    virtual void SetMesh(FRHICommandList& RHICmdList, FShader* PixelShader, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, uint32 DataFlags) const override;

    virtual uint32 GetSize() const override
    {
        return sizeof(*this);
    }

private:
    FShaderParameter LocalToWorldNoScalingParameter;
};

/** vertex factory for quady  */
class FQuadyVertexFactory 
    : public FVertexFactory
{
    DECLARE_VERTEX_FACTORY_TYPE(FQuadyVertexFactory);

public:
    FQuadyVertexFactory(ERHIFeatureLevel::Type InFeatureLevel);

    virtual ~FQuadyVertexFactory()
    {
        // can only be destroyed from the render thread
        ReleaseResource();
    }

    static FVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency);

    struct FDataType
    {
        /** The stream to read the vertex position from. */
        FVertexStreamComponent PositionComponent;
    };

    /**
    * Should we cache the material's shadertype on this platform with this vertex factory?
    */
    static bool ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FShaderType* ShaderType)
    {
        // only compile quady materials for quady vertex factory
        // The special engine materials must be compiled for the quady vertex factory because they are used with it for wireframe, etc.
        return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4) &&
            (Material->IsUsedWithLandscape() || Material->IsSpecialEngineMaterial());
    }

    /**
    * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
    */
    static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment);

    /**
    * Copy the data from another vertex factory
    * @param Other - factory to copy from
    */
    void Copy(const FQuadyVertexFactory& Other);

    // FRenderResource interface.
    virtual void InitRHI() override;

    static bool SupportsTessellationShaders() { return true; }

    /**
     * An implementation of the interface used by TSynchronizedResource to update the resource with new data from the game thread.
     */
    void SetData(const FDataType& InData)
    {
        Data = InData;
        UpdateRHI();
    }

    virtual uint64 GetStaticBatchElementVisibility(const FSceneView& InView, const FMeshBatch* InBatch, const void* InViewCustomData = nullptr) const override;

    /** stream component data bound to this vertex factory */
    FDataType Data;
};

/** vertex factory for quady  */
class FQuadyXYOffsetVertexFactory 
    : public FQuadyVertexFactory
{
    DECLARE_VERTEX_FACTORY_TYPE(FQuadyXYOffsetVertexFactory);

public:
    FQuadyXYOffsetVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
        : FQuadyVertexFactory(InFeatureLevel) { }

    virtual ~FQuadyXYOffsetVertexFactory() {}

    static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment);
};

struct FQuadyVertex
{
    float VertexX;
    float VertexY;
    float SubX;
    float SubY;
};

//
// FQuadyVertexBuffer
//
class FQuadyVertexBuffer 
    : public FVertexBuffer
{
private:
    ERHIFeatureLevel::Type FeatureLevel;
    int32 NumVertices;
    int32 SubsectionSizeVerts;
    int32 NumSubsections;

public:
    /** Constructor. */
    FQuadyVertexBuffer(ERHIFeatureLevel::Type InFeatureLevel, int32 InNumVertices, int32 InSubsectionSizeVerts, int32 InNumSubsections)
        : FeatureLevel(InFeatureLevel)
        , NumVertices(InNumVertices)
        , SubsectionSizeVerts(InSubsectionSizeVerts)
        , NumSubsections(InNumSubsections)
    {
        InitResource();
    }

    /** Destructor. */
    virtual ~FQuadyVertexBuffer()
    {
        ReleaseResource();
    }

    /**
    * Initialize the RHI for this rendering resource
    */
    virtual void InitRHI() override;
};

//
// FQuadySharedAdjacencyIndexBuffer
//
class FQuadySharedAdjacencyIndexBuffer 
    : public FRefCountedObject
{
public:
    FQuadySharedAdjacencyIndexBuffer(class FQuadySharedBuffers* SharedBuffer);
    virtual ~FQuadySharedAdjacencyIndexBuffer();

    TArray<FIndexBuffer*> IndexBuffers; // For tessellation
};

//
// FQuadySharedBuffers
//
class FQuadySharedBuffers 
    : public FRefCountedObject
{
public:
    struct FQuadyIndexRanges
    {
        int32 MinIndex[1][1];
        int32 MaxIndex[1][1];
        int32 MinIndexFull;
        int32 MaxIndexFull;
    };

    int32 NumVertices;
    int32 SharedBuffersKey;
    int32 NumIndexBuffers;
    int32 SubsectionSizeVerts;
    int32 NumSubsections;

    FQuadyVertexFactory* VertexFactory;
    FQuadyVertexBuffer* VertexBuffer;
    FIndexBuffer** IndexBuffers;
    FQuadyIndexRanges* IndexRanges;
    FQuadySharedAdjacencyIndexBuffer* AdjacencyIndexBuffers;
    FOccluderIndexArraySP OccluderIndicesSP;
    bool bUse32BitIndices;

    FQuadySharedBuffers(int32 SharedBuffersKey, int32 SubsectionSizeQuads, int32 NumSubsections, ERHIFeatureLevel::Type FeatureLevel, bool bRequiresAdjacencyInformation, int32 NumOcclusionVertices);
    virtual ~FQuadySharedBuffers();

    template <typename INDEX_TYPE>
    void CreateIndexBuffers(ERHIFeatureLevel::Type InFeatureLevel, bool bRequiresAdjacencyInformation);

    void CreateOccluderIndexBuffer(int32 NumOccluderVertices);
};

//
// FQuadyNeighborInfo
//
class FQuadyNeighborInfo
{
protected:
    static const int8 NEIGHBOR_COUNT = 4;

    // Key to uniquely identify the quad to find the correct render proxy map
    class FQuadyKey
    {
    private:
        const UWorld* World;
        const FGuid Guid;

    public:
        FQuadyKey(const UWorld* InWorld, const FGuid& InGuid)
            : World(InWorld),
            Guid(InGuid) { }

        friend inline uint32 GetTypeHash(const FQuadyKey& Key)
        {
            return HashCombine(GetTypeHash(Key.World), GetTypeHash(Key.Guid));
        }

        friend bool operator==(const FQuadyKey& A, const FQuadyKey& B)
        {
            return A.World == B.World && A.Guid == B.Guid;
        }
    };

    const FQuadyNeighborInfo* GetNeighbor(int32 Index) const
    {
        if (Index < NEIGHBOR_COUNT)
            return Neighbors[Index];

        return nullptr;
    }

    virtual const UQuadyMeshComponent* GetQuadyComponent() const { return nullptr; }

    // Map of currently registered quady proxies, used to register with our neighbors
    static TMap<FQuadyKey, TMap<FIntPoint, const FQuadyNeighborInfo*> > SharedSceneProxyMap;

    // For neighbor lookup
    FQuadyKey			LandscapeKey;
    FIntPoint				ComponentBase;

    // Pointer to our neighbor's scene proxies in NWES order (nullptr if there is currently no neighbor)
    mutable const FQuadyNeighborInfo* Neighbors[NEIGHBOR_COUNT];

    // Data we need to be able to access about our neighbor
    int8					ForcedLOD;
    int8					LODBias;
    bool					bRegistered;
    int32					PrimitiveCustomDataIndex;

    friend class FQuadyMeshComponentSceneProxy;

public:
    FQuadyNeighborInfo(const UWorld* World, const FGuid& Guid, const FIntPoint& ComponentBase, int8 ForcedLOD, int8 LODBias)
        : LandscapeKey(World, Guid)
        , ComponentBase(ComponentBase)
        , ForcedLOD(ForcedLOD)
        , LODBias(LODBias)
        , bRegistered(false)
        , PrimitiveCustomDataIndex(INDEX_NONE)
    {
        //       -Y       
        //    - - 0 - -   
        //    |       |   
        // -X 1   P   2 +X
        //    |       |   
        //    - - 3 - -   
        //       +Y       

        Neighbors[0] = nullptr;
        Neighbors[1] = nullptr;
        Neighbors[2] = nullptr;
        Neighbors[3] = nullptr;
    }

    void RegisterNeighbors();
    void UnregisterNeighbors();
};
