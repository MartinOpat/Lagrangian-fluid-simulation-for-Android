precision mediump float;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle
uniform sampler2D uTexture; // Texture for the point sprite

void main() {
    if (uIsPoint) {
        gl_FragColor = texture2D(uTexture, gl_PointCoord);
    } else {
        gl_FragColor = vec4(0.4, 0.5, 0.8, 1.0);  // Blue for now
    }
}

//precision mediump float;
//uniform vec4 u_Color;
//void main() {
//    gl_FragColor = u_Color;
//}
