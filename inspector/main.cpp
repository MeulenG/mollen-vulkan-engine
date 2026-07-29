// mollen-inspector: developer tool for inspecting WoW file formats.
//
// Standalone ImGui window (software-rendered via imgui_impl_opengl3 is not
// available here since we use the Vulkan backend in the engine, so we use a
// minimal GLFW + OpenGL3 setup that doesn't require the full Vulkan stack).
//
// What it does:
//   - Accept a file path (typed or drag-dropped)
//   - Walk the chunk stream and display every chunk ID, size, offset
//   - For known chunks (MVER, MCNK, MAIN...) decode and display fields
//   - Show raw hex for unknown chunks
//   - Let you validate struct layout assumptions at runtime

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// File reading
// ---------------------------------------------------------------------------

static std::vector<uint8_t> ReadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return {};
    auto sz = f.tellg();
    f.seekg(0);
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    f.read(reinterpret_cast<char*>(buf.data()), sz);
    return buf;
}

static uint32_t U32At(const std::vector<uint8_t>& buf, size_t off) {
    if (off + 4 > buf.size()) return 0;
    uint32_t v = 0;
    std::memcpy(&v, buf.data() + off, 4);
    return v;
}

static float F32At(const std::vector<uint8_t>& buf, size_t off) {
    float v = 0;
    std::memcpy(&v, buf.data() + off, 4);
    return v;
}

// ---------------------------------------------------------------------------
// Chunk walking
// ---------------------------------------------------------------------------

struct ChunkEntry {
    char     id_ascii[5]; // null-terminated human-readable (reversed from disk)
    uint32_t id_raw;
    uint32_t size;
    size_t   offset;      // offset of payload in file
};

// Known on-disk byte patterns and their human-readable names.
// Bytes are as they appear on disk (reversed from the logical name).
static const char* ChunkName(const uint8_t* p) {
    if (p[0]==0x52 && p[1]==0x45 && p[2]==0x56 && p[3]==0x4D) return "MVER";
    if (p[0]==0x44 && p[1]==0x48 && p[2]==0x50 && p[3]==0x4D) return "MPHD";
    if (p[0]==0x4E && p[1]==0x49 && p[2]==0x41 && p[3]==0x4D) return "MAIN";
    if (p[0]==0x4F && p[1]==0x4D && p[2]==0x57 && p[3]==0x4D) return "MWMO";
    if (p[0]==0x46 && p[1]==0x44 && p[2]==0x4F && p[3]==0x4D) return "MODF";
    if (p[0]==0x58 && p[1]==0x45 && p[2]==0x54 && p[3]==0x4D) return "MTEX";
    if (p[0]==0x4B && p[1]==0x4E && p[2]==0x43 && p[3]==0x4D) return "MCNK";
    if (p[0]==0x54 && p[1]==0x56 && p[2]==0x43 && p[3]==0x4D) return "MCVT";
    if (p[0]==0x59 && p[1]==0x4C && p[2]==0x43 && p[3]==0x4D) return "MCLY";
    if (p[0]==0x52 && p[1]==0x44 && p[2]==0x48 && p[3]==0x4D) return "MHDR";
    if (p[0]==0x4E && p[1]==0x49 && p[2]==0x43 && p[3]==0x4D) return "MCIN";
    return nullptr;
}

static std::vector<ChunkEntry> WalkChunks(const std::vector<uint8_t>& buf) {
    std::vector<ChunkEntry> chunks;
    size_t pos = 0;
    while (pos + 8 <= buf.size()) {
        ChunkEntry e;
        e.id_raw = U32At(buf, pos);
        e.size   = U32At(buf, pos + 4);
        e.offset = pos + 8;

        const char* name = ChunkName(buf.data() + pos);
        if (name) {
            std::strncpy(e.id_ascii, name, 4);
        } else {
            // Show raw bytes as printable ASCII where possible.
            for (int i = 0; i < 4; i++) {
                uint8_t b = buf[pos + i];
                e.id_ascii[i] = (b >= 32 && b < 127) ? (char)b : '?';
            }
        }
        e.id_ascii[4] = '\0';
        chunks.push_back(e);

        if (e.offset + e.size > buf.size()) break;
        pos = e.offset + e.size;
    }
    return chunks;
}

// ---------------------------------------------------------------------------
// Chunk detail panels
// ---------------------------------------------------------------------------

static void ShowMVER(const std::vector<uint8_t>& buf, const ChunkEntry& e) {
    if (e.size < 4) { ImGui::TextDisabled("(payload too small)"); return; }
    uint32_t ver = U32At(buf, e.offset);
    ImGui::Text("version: %u", ver);
    if (ver == 18)
        ImGui::TextColored({0,1,0,1}, "OK - WotLK/v18");
    else
        ImGui::TextColored({1,0.4f,0,1}, "WARNING: unexpected version (expected 18 for WotLK)");
}

static void ShowMAIN(const std::vector<uint8_t>& buf, const ChunkEntry& e) {
    constexpr int kGrid = 64;
    if (e.size < static_cast<uint32_t>(kGrid * kGrid * 8)) {
        ImGui::TextDisabled("(payload too small for 64x64 grid)");
        return;
    }
    int count = 0;
    for (int y = 0; y < kGrid; y++)
        for (int x = 0; x < kGrid; x++) {
            uint32_t flags = U32At(buf, e.offset + (y * kGrid + x) * 8);
            if (flags & 0x1) count++;
        }
    ImGui::Text("Existing tiles: %d / %d", count, kGrid * kGrid);

    if (ImGui::TreeNode("Tile map")) {
        for (int y = 0; y < kGrid; y++) {
            for (int x = 0; x < kGrid; x++) {
                uint32_t flags = U32At(buf, e.offset + (y * kGrid + x) * 8);
                ImGui::SameLine(0, 0);
                ImGui::TextColored(
                    (flags & 0x1) ? ImVec4{0,1,0,1} : ImVec4{0.2f,0.2f,0.2f,1},
                    (flags & 0x1) ? "#" : ".");
            }
            ImGui::NewLine();
        }
        ImGui::TreePop();
    }
}

static void ShowMCNK(const std::vector<uint8_t>& buf, const ChunkEntry& e) {
    if (e.size < 128) { ImGui::TextDisabled("(header too small)"); return; }
    size_t o = e.offset;
    ImGui::Text("IndexX:       %u", U32At(buf, o + 0x04));
    ImGui::Text("IndexY:       %u", U32At(buf, o + 0x08));
    ImGui::Text("nLayers:      %u", U32At(buf, o + 0x0C));
    ImGui::Text("nDoodadRefs:  %u", U32At(buf, o + 0x10));
    ImGui::Text("ofsHeight:    0x%04X", U32At(buf, o + 0x14));
    ImGui::Text("ofsNormal:    0x%04X", U32At(buf, o + 0x18));
    ImGui::Text("ofsLayer:     0x%04X", U32At(buf, o + 0x1C));
    ImGui::Text("holes_lo:     0x%04X", (uint32_t)(buf[o + 0x3C] | (buf[o + 0x3D] << 8)));
    ImGui::Text("position:     (%.2f, %.2f, %.2f)",
        F32At(buf, o + 0x68), F32At(buf, o + 0x6C), F32At(buf, o + 0x70));
}

static void ShowHex(const std::vector<uint8_t>& buf, const ChunkEntry& e) {
    size_t show = std::min((size_t)e.size, (size_t)128);
    std::string hex;
    hex.reserve(show * 3);
    for (size_t i = 0; i < show; i++) {
        char tmp[4];
        std::snprintf(tmp, sizeof(tmp), "%02X ", buf[e.offset + i]);
        hex += tmp;
        if ((i + 1) % 16 == 0) hex += '\n';
    }
    if (e.size > 128) hex += "\n... (truncated)";
    ImGui::TextUnformatted(hex.c_str());
}

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------

struct AppState {
    char path_buf[512] = {};
    std::vector<uint8_t> file_data;
    std::vector<ChunkEntry> chunks;
    int selected = -1;
    std::string error;

    void Load() {
        error.clear();
        file_data = ReadFile(path_buf);
        if (file_data.empty()) {
            error = "Could not open file: ";
            error += path_buf;
            chunks.clear();
            selected = -1;
            return;
        }
        chunks = WalkChunks(file_data);
        selected = -1;
    }
};

// ---------------------------------------------------------------------------
// Main UI
// ---------------------------------------------------------------------------

static void DrawUI(AppState& state) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("mollen-inspector", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open...")) { /* TODO: native file dialog */ }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Path input
    ImGui::SetNextItemWidth(-80);
    ImGui::InputText("##path", state.path_buf, sizeof(state.path_buf));
    ImGui::SameLine();
    if (ImGui::Button("Load")) state.Load();

    // Drag-drop onto window
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("text/uri-list")) {
            std::string uri(static_cast<const char*>(p->Data), p->DataSize);
            if (uri.rfind("file:///", 0) == 0) uri = uri.substr(8);
            std::strncpy(state.path_buf, uri.c_str(), sizeof(state.path_buf) - 1);
            state.Load();
        }
        ImGui::EndDragDropTarget();
    }

    if (!state.error.empty()) {
        ImGui::TextColored({1,0.3f,0.3f,1}, "%s", state.error.c_str());
    }

    if (state.file_data.empty()) {
        ImGui::End();
        return;
    }

    ImGui::Text("File size: %zu bytes  |  Chunks: %zu",
        state.file_data.size(), state.chunks.size());
    ImGui::Separator();

    // Left panel: chunk list
    float list_w = 220.0f;
    ImGui::BeginChild("##chunks", {list_w, 0}, true);
    for (int i = 0; i < (int)state.chunks.size(); i++) {
        const auto& c = state.chunks[i];
        char label[64];
        std::snprintf(label, sizeof(label), "%-4s  +0x%05zX  (%u B)",
            c.id_ascii, c.offset - 8, c.size);
        if (ImGui::Selectable(label, state.selected == i))
            state.selected = i;
    }
    ImGui::EndChild();

    // Right panel: chunk detail
    ImGui::SameLine();
    ImGui::BeginChild("##detail", {0, 0}, true);
    if (state.selected >= 0 && state.selected < (int)state.chunks.size()) {
        const auto& e = state.chunks[state.selected];
        ImGui::Text("Chunk: %s", e.id_ascii);
        ImGui::Text("Offset: +0x%05zX  Size: %u bytes", e.offset - 8, e.size);
        ImGui::Separator();

        if      (std::strcmp(e.id_ascii, "MVER") == 0) ShowMVER(state.file_data, e);
        else if (std::strcmp(e.id_ascii, "MAIN") == 0) ShowMAIN(state.file_data, e);
        else if (std::strcmp(e.id_ascii, "MCNK") == 0) ShowMCNK(state.file_data, e);
        else { ImGui::TextDisabled("No decoder yet. Raw bytes:"); ShowHex(state.file_data, e); }

        ImGui::Separator();
        if (ImGui::CollapsingHeader("Raw hex")) ShowHex(state.file_data, e);
    } else {
        ImGui::TextDisabled("Select a chunk on the left.");
    }
    ImGui::EndChild();
    ImGui::End();
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "mollen-inspector", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    AppState state;
    if (argc > 1) {
        std::strncpy(state.path_buf, argv[1], sizeof(state.path_buf) - 1);
        state.Load();
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawUI(state);

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
