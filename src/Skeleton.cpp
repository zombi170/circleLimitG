//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Adam Zsombor
// Neptun : X079FB
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char * const vertexSource = R"(
	#version 330
	precision highp float;

	uniform mat4 MVP;
	layout(location = 0) in vec2 vp;
    layout(location = 1) in vec2 vertexUV;
    out vec2 texC;

	void main() {
        texC = vertexUV;
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;
	}
)";

const char * const fragmentSource = R"(
	#version 330
	precision highp float;
	
	uniform sampler2D texU;
    in vec2 texC;
	out vec4 outColor;

	void main() {
		outColor = texture(texU, texC);
	}
)";

float toRadian(int degree) {
    return M_PI * degree / 180.0f;
}

class Circle {
    vec2 center;
    float radius;
    
public:
    
    Circle(vec2 c, float r) : center(c), radius(r) {}
    
    bool inCircle(vec2 point) {
        return length(point - center) < radius ? true : false;
    }
};

class PoincareTexture {
    Circle basic;
    std::vector<Circle> circles;
    
    float toPoincareDistance(float distanceH) {
        return sinhf(distanceH) / (1.0f + coshf(distanceH));
    }
    
    vec2 getCirclePoint(float distance, int degree) {
        float radian = toRadian(degree);
        float distanceP = toPoincareDistance(distance);
        return vec2(distanceP * cosf(radian), distanceP * sinf(radian));
    }
    
    float squaredLength(vec2 vector) {
        return pow(vector.x, 2) + pow(vector.y, 2);
    }
    
    vec2 getInvertPoint(vec2 point) {
        return vec2(point.x / squaredLength(point), point.y / squaredLength(point));
    }
    
    float sumHalf(float a, float b) {
        return (a + b) / 2;
    }
    
    vec2 getCircleCenter(float distance, int degree) {
        vec2 circlePoint = getCirclePoint(distance, degree);
        vec2 invertPoint = getInvertPoint(circlePoint);
        return vec2(sumHalf(circlePoint.x, invertPoint.x),
                    sumHalf(circlePoint.y, invertPoint.y));
    }
    
    float getRadius(vec2 p1, vec2 p2) {
        return length(p1 - p2);
    }
    
    void createCircles() {
        for (int de = 0; de <= 320; de += 40) {
            for (float di = 0.5f; di <= 5.5f; di++) {
                vec2 circlePoint = getCirclePoint(di, de);
                vec2 center = getCircleCenter(di, de);
                float radius = getRadius(circlePoint, center);
                circles.push_back(Circle(center, radius));
            }
        }
    }
    
    vec4 decideColor(vec2 point) {
        int counter = 0;
        
        for (Circle circle : circles) {
            if (basic.inCircle(point)) {
                if (circle.inCircle(point)) {
                    counter++;
                }
            } else {
                return vec4(0, 0, 0, 1);
            }
        }
        
        if (counter % 2 == 0) {
            return vec4(1, 1, 0, 1);
        }
        
        return vec4(0, 0, 1, 1);
    }
    
public:
    
    PoincareTexture() : basic(Circle(vec2(0, 0), 1.0f)) {
        createCircles();
    }
    
    std::vector<vec4> renderToTexture(int width, int height) {
        std::vector<vec4> image(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                
                float cX = 2.0f * x / width - 1;
                float cY = 1.0f - 2.0f * y / height;
                vec2 point = vec2(cX, cY);
                
                image[y * width + x] = decideColor(point);
            }
        }
        return image;
    }
};

class Camera {
    
    float centerX, centerY, worldWidth, worldHeight;
    
public:
    
    Camera() {
        centerX = 20;
        centerY = 30;
        worldWidth = 150;
        worldHeight = 150;
    }

    mat4 view() {
        return TranslateMatrix(vec3 (-centerX, -centerY, 0));
    }

    mat4 projection() {
        return ScaleMatrix(vec3 (2 / worldWidth, 2 / worldHeight, 1));
    }
    
};

GPUProgram gpuProgram;
Camera camera = Camera();
PoincareTexture* poincare;
float tNow;

class Star {
    Texture texture;
    vec2 texPoints[9], uvs[9];
    unsigned int vao, vbo[2];
    std::vector<vec2> points;
    vec2 center;
    float corner, s;
    int filter, resolution;
    mat4 matrix;
    
    void calculatePoints() {
        points.clear();
        points.push_back(center);
        points.push_back(vec2(center.x - corner, center.y - corner));
        points.push_back(vec2(center.x - s, center.y));
        points.push_back(vec2(center.x - corner, center.y + corner));
        points.push_back(vec2(center.x, center.y + s));
        points.push_back(vec2(center.x + corner, center.y + corner));
        points.push_back(vec2(center.x + s, center.y));
        points.push_back(vec2(center.x + corner, center.y - corner));
        points.push_back(vec2(center.x, center.y - s));
        points.push_back(vec2(center.x - corner, center.y - corner));
    }
    
public:
    
    Star() {
        texPoints[0] = vec2(50, 30);
        texPoints[1] = vec2(10, -10);
        texPoints[2] = vec2(10, 30);
        texPoints[3] = vec2(10, 70);
        texPoints[4] = vec2(50, 70);
        texPoints[5] = vec2(90, 70);
        texPoints[6] = vec2(90, 30);
        texPoints[7] = vec2(90, -10);
        texPoints[8] = vec2(50, -10);
        uvs[0] = vec2(0.5f, 0.5f);
        uvs[1] = vec2(0, 0);
        uvs[2] = vec2(0, 0.5f);
        uvs[3] = vec2(0, 1);
        uvs[4] = vec2(0.5f, 1);
        uvs[5] = vec2(1, 1);
        uvs[6] = vec2(1, 0.5f);
        uvs[7] = vec2(1, 0);
        uvs[8] = vec2(0.5f, 0);
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texPoints), texPoints, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        
        center = texPoints[0];
        corner = 40;
        s = 40;
        filter = GL_LINEAR;
        resolution = 300;
        matrix = camera.view() * camera.projection();
    }
    
    void draw() {
        calculatePoints();
        texture.create(resolution, resolution,
                       poincare->renderToTexture(resolution, resolution), filter);
        
        gpuProgram.setUniform(matrix, "MVP");
        gpuProgram.setUniform(texture, "texU");
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec2), &points[0], GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_FAN, 0, (int)points.size());
    }
    
    float getS() {
        return s;
    }
    
    void setS(float s) {
        this->s = s;
    }
    
    void setFilter(int f) {
        this->filter = f;
    }
    
    int getResolution() {
        return resolution;
    }
    
    void setResolution(int r) {
        this->resolution = r;
    }

    void orbit() {
        std::vector<vec2> orbit;
        int n = 500;
        for (int i = 0; i < n; i++) {
            float alpha = (i * 2 * M_PI) / n;
            float x = cosf(alpha) * 30 + 20;
            float y = sinf(alpha) * 30 + 30;
            orbit.push_back(vec2(x, y));
        }
        
        float last = orbit.size() - 1;
        float t = tNow * n / 10;
        while (t > last) {
            t -= last;
        }
        center = orbit[t];
    }
    
    void rotate() {
        float alpha = tNow * toRadian(72);
        matrix = TranslateMatrix(vec3(-center.x, -center.y, 0)) *
            RotationMatrix(alpha, vec3(0, 0, 1)) *
            TranslateMatrix(vec3(center.x, center.y, 0)) *
            camera.view() * camera.projection();
    }
    
    void animate() {
        orbit();
        rotate();
    }
};

Star* star;
int a = -1;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
    poincare = new PoincareTexture();
    star = new Star();
}

void onDisplay() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
    if (a != -1) {
        star->animate();
    }
    star->draw();
	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
    switch (key) {
        case 'h':
            star->setS(star->getS() - 10);
            break;
        case 'r':
            star->setResolution(star->getResolution() - 100);
            break;
        case 'R':
            star->setResolution(star->getResolution() + 100);
            break;
        case 't':
            star->setFilter(GL_NEAREST);
            break;
        case 'T':
            star->setFilter(GL_LINEAR);
            break;
        case 'a':
            a = glutGet(GLUT_ELAPSED_TIME);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void onKeyboardUp(unsigned char key, int pX, int pY) {}

void onMouseMotion(int pX, int pY) {}

void onMouse(int button, int state, int pX, int pY) {}

void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME);
    tNow = (time - a) / 1000.0f;
    glutPostRedisplay();
}
