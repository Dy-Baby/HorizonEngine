#include "SceneView.h"
#include "RenderBackend.h"
#include "RenderContext.h"
#include "RenderCommandList.h"
#include "RenderPipeline.h"
#include "RenderGraph/RenderGraph.h"

namespace HE
{

void UpdateCameraMatrices(Camera& camera, float aspectRatio)
{
    /*Quaternion rotation = Quaternion(Math::DegreesToRadians(camera.euler));
    static Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
    camera.viewMatrix = Math::Inverse(Math::Compose(camera.position, rotation, scale));
    camera.projectionMatrix = glm::perspective(Math::DegreesToRadians(camera.fieldOfView), aspectRatio, camera.zNear, camera.zFar);*/
}

void RenderSceneView(
    RenderContext* renderContext,
    SceneView* view)
{
    MemoryArena* arena = renderContext->arena;
    RenderBackend* renderBackend = renderContext->renderBackend;
    RenderPipeline* activePipeline = view->renderPipeline;
    uint32 deviceMask = ~0u;

    UpdateCameraMatrices(view->camera, (float)(view->targetWidth / view->targetHeight));
    {
        RenderGraph renderGraph(arena);
        
        activePipeline->SetupRenderGraph(view, &renderGraph);
        
        renderContext->commandLists.clear();
        renderGraph.Execute(renderContext);

        if (false)
        {
            HE_LOG_INFO(renderGraph.Graphviz());
        }
    }

    uint32 numCommandLists = (uint32)renderContext->commandLists.size();
    RenderCommandList** commandLists = renderContext->commandLists.data();
    SubmitRenderCommandLists(renderBackend, commandLists, numCommandLists);
}

}