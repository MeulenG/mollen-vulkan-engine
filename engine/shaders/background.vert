#version 450

// Fullscreen triangle — no vertex buffer needed.
// Draws a single triangle that covers the entire screen.
// gl_VertexIndex 0,1,2 produces coordinates that cover [-1,1] NDC range.

layout(location = 0) out vec2 frag_uv;

void main() {
    // Generate fullscreen triangle vertices from vertex index:
    //   0: (-1, -1)  →  UV (0, 0)  bottom-left
    //   1: ( 3, -1)  →  UV (2, 0)  far right
    //   2: (-1,  3)  →  UV (0, 2)  far top
    // The triangle is oversized but gets clipped to the viewport.
    frag_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(frag_uv * 2.0 - 1.0, 0.9999, 1.0); // z=0.9999 = far back
}
