//attribute vec4 vPosition;
//uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle
//uniform vec3 uPosition;
//
//void main() {
////    gl_Position = vPosition;
////    vec3 pos = vPosition.xyz;
//
//    if (uIsPoint) {
//        gl_PointSize = 100.0;
////        pos += uVelocity * uTime;
////
////        // Wrap x coordinate
////        if (pos.x > 1.0) pos.x = -1.0;
////        else if (pos.x < -1.0) pos.x = 1.0;
////
////        // Wrap y coordinate
////        if (pos.y > 1.0) pos.y = 1.0;
////        else if (pos.y < -1.0) pos.y = -1.0;
//        gl_Position = vec4(uPosition, 1.0);
//
//    } else {
//        gl_Position = vPosition;
//    }
//
//}


attribute vec4 a_Position;
void main() {
    gl_Position = a_Position;
}
