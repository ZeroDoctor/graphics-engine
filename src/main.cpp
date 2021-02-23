#include "build_order.hpp"
#include "render_manager.hpp"
#include "swapchain.hpp"
#include <unistd.h>

uint32_t id = 0; // shape id

std::vector<Vertex> vertices;
std::vector<uint16_t> indices;

// 72 pixel/inch which is 595x842
// 96 p/i 794x1123
// 150 pi/i 1240x1754
// 300 pi/i which is 2480x3508
const uint32_t width = 794;
const uint32_t height = 1123;
const float wf = static_cast<float>(width);
const float hf = static_cast<float>(height);
const glm::mat4 projOrtho = glm::ortho(0.0f, wf, hf, 0.0f,-5.0f, 5.0f); // we can do this in the fragment shader instead

void draw_line(float x, float y, float length, float size=2, float angle=0) 
{
    glm::vec4 v1 = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec4 v2 = {0.0f, size, 0.0f, 1.0f};
    glm::vec4 v3 = {length, size, 0.0f, 1.0f};
    glm::vec4 v4 = {length, 0.0f, 0.0f, 1.0f};


    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

    const Vertex verts[4] = {
        {projOrtho * model * v1, {0.0f, 0.0f, 0.0f}}, // hard coded colors for now
        {projOrtho * model * v2, {0.0f, 0.0f, 0.0f}},
        {projOrtho * model * v3, {0.0f, 0.0f, 0.0f}},
        {projOrtho * model * v4, {0.0f, 0.0f, 0.0f}}
    };

    vertices.insert(vertices.end(), verts, verts+4);

    uint16_t tr = static_cast<uint16_t>(id);
    uint16_t br = static_cast<uint16_t>(id + 1);
    uint16_t bl = static_cast<uint16_t>(id + 2);
    uint16_t tl = static_cast<uint16_t>(id + 3);
    const uint16_t pos[6] = {
        tr, br, bl, bl, tl, tr
    };

    indices.insert(indices.end(), pos, pos+6);

    id+=4;
}

void draw_box(float x, float y, float w, float h, float size=2) 
{   
    draw_line(x, y, w, size, 0);
    draw_line(x+w-size, y, h, size, -90);
    draw_line(x+w-size, y-h+size, w, size, 180);
    draw_line(x, y-h+size, h, size, 90);
}

int main() 
{

    const uint32_t swidth = 1280;
    const uint32_t sheight = 720;
    VulkanConfiguration config;
    config.application_name = "Sup";
    config.application_version = VK_MAKE_VERSION(1,0,0);

    WindowSettings settings;
    settings.fullscreen = false;
    settings.title = config.application_name;

    VulkanInstance instance(config);
    VulkanSurface surface(&instance, settings, swidth, sheight);
    VulkanPhysicalDevice* physical = VulkanPhysicalDevice::GetPhysicalDevice(&instance, &surface, true);
    VulkanDevice device(&instance, physical);

    VulkanSwapChain swapchain(&instance, physical, &device, &surface, swidth, sheight);
    VulkanGraphicsPipline pipeline(&device, swidth, sheight);
    std::vector<VkImageView> image_views = swapchain.GetImageViews();

    VkFormat depth_format;
    if(!device.GetSupportedDepthFormat(&depth_format)) {
        printff("failed to find depth supported logical device");
    }

    VulkanImageView* depth_view = new VulkanImageView(&device);
    depth_view->GenerateImage(swidth, sheight, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageAspectFlags flags[] = {
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
    };
    depth_view->CreateImageView(flags);

    draw_box(wf/2 - (300/2), hf/2 + (250/2), 300, 250, 5);
    draw_box(wf/2 - (600/2), hf/2 + (500/2), 600, 500, 5);

    std::unique_ptr<VulkanVertexBuffer> vertex_buffer(new VulkanVertexBuffer(
        &device, vertices, indices
    ));

    // VkCommandBuffer* command = new VkCommandBuffer[image_views.size()];

    // pipeline
    {
        pipeline.CreateShaderModule("./../src/shader/vert.spv", "./../src/shader/frag.spv");
        pipeline.CreateRenderPass(swapchain.GetFormat(), depth_format, true);
        pipeline.CreateFrameBuffers(image_views.size(), image_views, &depth_view->GetImageViews()[0]);
        // pipeline.CreatePipelineLayout(swidth, sheight);
        // pipeline.CreateCommandBuffers(command, image_views.size(), vertex_buffer.get());
    }

    // device.FreeComputeCommand(command, image_views.size());

    delete depth_view;

    printfi("--> program endo...\n");
    return 0;

    const VkFormat src_format = VK_FORMAT_R8G8B8A8_UNORM; // SRGB

    printfi("--> program starto...\n");

    draw_box(wf/2 - (300/2), hf/2 + (250/2), 300, 250, 5);
    draw_box(wf/2 - (600/2), hf/2 + (500/2), 600, 500, 5);

    std::unique_ptr<RenderManager> renderer(new RenderManager());

    renderer->Init("Graphics Library");
    
    renderer->Setup(width, height, src_format);

    // create output image
    VulkanImageView* output_view = renderer->Draw(vertices, indices);

    renderer->Close();
    
    // save file
    renderer->SaveImage("another.ppm", output_view);
    delete output_view;

    renderer->Wait();
    
    printfi("--> program endo...\n");
    return EXIT_SUCCESS;
}