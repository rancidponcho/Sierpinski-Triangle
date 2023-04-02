#include "first_app.hpp"

// std
#include <stdexcept>

namespace sve {

FirstApp::FirstApp() {
    loadModels();
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

FirstApp::~FirstApp() {
    vkDestroyPipelineLayout(sveDevice.device(), pipelineLayout, nullptr);
}

void FirstApp::run() {
    while (!sveWindow.shouldClose()) {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(sveDevice.device());
}

void FirstApp::loadModels() {
    std::vector<SveModel::Vertex> vertices{};
    SierpinskiTriangle(vertices, 6, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
    sveModel = std::make_unique<SveModel>(sveDevice, vertices);
}

void FirstApp::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;             // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;          // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;     // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr;  // Optional

    if (vkCreatePipelineLayout(sveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void FirstApp::createPipeline() {
    auto pipelineConfig = SvePipeline::defaultPipelineConfigInfo(sveSwapChain.width(), sveSwapChain.height());
    pipelineConfig.renderPass = sveSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    svePipeline = std::make_unique<SvePipeline>(
        sveDevice,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        pipelineConfig);
}

void FirstApp::createCommandBuffers() {
    commandBuffers.resize(sveSwapChain.imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = sveDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(sveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (int i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                   // Optional
        beginInfo.pInheritanceInfo = nullptr;  // Optional

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = sveSwapChain.getRenderPass();
        renderPassInfo.framebuffer = sveSwapChain.getFrameBuffer(i);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = sveSwapChain.getSwapChainExtent();

        // Attachment index specified in the renderpass
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        svePipeline->bind(commandBuffers[i]);
        sveModel->bind(commandBuffers[i]);
        sveModel->draw(commandBuffers[i]);

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void FirstApp::drawFrame() {
    uint32_t imageIndex;
    auto result = sveSwapChain.acquireNextImage(&imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    result = sveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

// MISC.
void FirstApp::SierpinskiTriangle(std::vector<SveModel::Vertex> &vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top) {
    if (depth <= 0) {
        vertices.push_back({top});
        vertices.push_back({right});
        vertices.push_back({left});
    } else {
        auto leftTop = 0.5f * (left + top);
        auto rightTop = 0.5f * (right + top);
        auto leftRight = 0.5f * (left + right);
        SierpinskiTriangle(vertices, depth - 1, left, leftRight, leftTop);
        SierpinskiTriangle(vertices, depth - 1, leftRight, right, rightTop);
        SierpinskiTriangle(vertices, depth - 1, leftTop, rightTop, top);
    }
}

}  // namespace sve