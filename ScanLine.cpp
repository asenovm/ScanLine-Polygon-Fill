#include "stdafx.h"
#include <windows.h>   // use as needed for your system
#include <GL/GL.h>
#include "glut.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <math.h>
#include <stdlib.h>
#include <GdiPlus.h>
#pragma comment(lib,"gdiplus.lib")
using namespace std;
using namespace Gdiplus;
#define ScreenWidth 640
#define ScreenHeight 480
#define MAX_GLPoint_COUNT 32
#define PI 3.141592
int win_height = ScreenHeight;
int win_width = ScreenWidth;
// flag according to which we decide what to draw and what not
bool flag_rotate = false;
//some function declarations
void RotatePolygon(double,bool);
float signedArea(void);
void drawGLPoints(void);
void drawEdges(bool);
// the number of points of the polygon we've put so far
int current = 0;
//is it time to fill the polygon?
bool flag = false;
//constructing class to represent a point on the screen
class GLPoint {
public:
	int x;
	int y;
	GLPoint(int cx = 0, int cy = 0) : x(cx), y(cy) {}
}; 
GLPoint oo;
//method declaration for the method returning the mass center/centroid of the polygon
GLPoint getCentroid(void);
//class to represent the edges of the polygon
class Line {
public:
	GLPoint p1;
	GLPoint p2;
	Line(GLPoint cp1 = oo, GLPoint cp2 = oo) : p1(cp1), p2(cp2) {}
}; 
//initializing point_ymin and point_ymax so that point_ymin is always bigger than the y coordinate of all the points of the polygon
//and point y_max is always less than the y coordinate of all the points in the polygon. After going through all the vertecies of 
//the polygon point_ymin will be least y_coordinate of a point(where the scanline will start) and point_ymax will be the biggest
//y coordinate of a point(where the scanline will end)
int point_ymin = 2000;
int point_ymax = 0;
//vector of points to represent the points of the polygon, the intersection points of each scanline with all the edges of the polygon
//and all the edges of the polygon
vector<GLPoint> polygon;
vector<GLPoint> ListOfIntersectPoints;
vector<Line> ListOfEdges;
// (x2-x1)/(y2-y1) = ctg(alpha) where alpha is the angle between the line || to the y axis going through the GLPoint(x1,y1)
// it's easy to see(trigonometry!) that x is exactly the x coordinate of crossing point of the line y with the edge
// (x1,y1)(x2,y2) of the polygon. If y1==y2(horizontal edge) we don't count any points of the edge as intersecting with the scanline
bool findIntersectGLPoint(int x1, int y1, int x2, int y2, int y, int &x) {
	if(y2==y1)
		return false;
	x = (x2-x1)*(y-y1)/(y2-y1) + x1;
	bool isInsideEdgeX;
	bool isInsideEdgeY;
	if(x1 < x2) 
		isInsideEdgeX = (x1 <= x) && (x <= x2);
	else 
		isInsideEdgeX = (x2 <= x) && (x <= x1);

	if(y1 < y2)
		isInsideEdgeY = (y1 <= y) && (y <= y2);
	else
		isInsideEdgeY = (y2 <= y) && (y <= y1);
	return isInsideEdgeX && isInsideEdgeY;
}
//fill the ListOfEdges vector with the coordinates of the edges of the polygon. Find the point_ymin where the scanline
//will start and point_ymax where the scanline will end
void initEdges()
{
	if(polygon.size() > 2)
	{
		for(int i = 1;i<polygon.size();i++)
		{
			if(polygon.at(i-1).y < point_ymin)
				point_ymin = polygon.at(i-1).y;
			if(polygon.at(i-1).y > point_ymax)
				point_ymax = polygon.at(i-1).y;
			Line current = Line(polygon.at(i-1),polygon.at(i));
			ListOfEdges.push_back(current);
		}
		int i = polygon.size()-1;
		if(polygon.at(i).y > point_ymax)
			point_ymax = polygon.at(i).y;
		if(polygon.at(i).y < point_ymin)
			point_ymin = polygon.at(i).y;
		Line last = Line(polygon.at(i),polygon.at(0));
		ListOfEdges.push_back(last);
	}
}
//For each scanline (line with equation y = i where point_ymin <= y <= point_ymax) we find and store all the intersect points of the 
//scanline with all the edges of the polygon. Afterwards sort these points according to their x coords and draw a line between each
//two of them. Special cases: when the scanline goes through an edge of the polygon. Then this edge will be counted twice(as intersection
//point of the scanline with two different sides of the polygon) --> we want to count it only once, so we count it only as the y_min
//coordinate of some side
void scanlineFill()
{			
			int edgesSize = ListOfEdges.size();
			for(int i = point_ymin; i <= point_ymax; i++) {
				int intersectX;
				for(int j = 0; j < edgesSize; j++) {
					if (findIntersectGLPoint(ListOfEdges.at(j).p1.x, ListOfEdges.at(j).p1.y, ListOfEdges.at(j).p2.x, ListOfEdges.at(j).p2.y, i, intersectX))
						{
							GLPoint p(intersectX, i);
							if(ListOfEdges[j].p1.y > ListOfEdges[j].p2.y)
							{
								if(p.y == ListOfEdges[j].p1.y)
									continue;
							}
							else
								if(ListOfEdges[j].p1.y < ListOfEdges[j].p2.y)
								{
									if(p.y == ListOfEdges[j].p2.y)
										continue;
								}
							ListOfIntersectPoints.push_back(p);
						}
				}
				int intersectSize = ListOfIntersectPoints.size();
				GLPoint swap = GLPoint(0,0);
				for(int i = 0;i<intersectSize-1;i++)
					for(int j = i+1;j<intersectSize;j++)
					{
						if(ListOfIntersectPoints[i].x > ListOfIntersectPoints[j].x)
						{
							swap = ListOfIntersectPoints[i];
							ListOfIntersectPoints[i] = ListOfIntersectPoints[j];
							ListOfIntersectPoints[j] = swap;
						}
					}
				int intersectPointsSize = ListOfIntersectPoints.size();
				for(int j = 1; j < intersectPointsSize; j+=2) {

					glBegin(GL_LINES);
						glVertex2i(ListOfIntersectPoints.at(j-1).x, ListOfIntersectPoints.at(j-1).y);
						glVertex2i(ListOfIntersectPoints.at(j).x, ListOfIntersectPoints.at(j).y);
					glEnd();

				}
				ListOfIntersectPoints.erase(ListOfIntersectPoints.begin(),ListOfIntersectPoints.end());
			}
}
// checking if the points added by the mouse are ok(no self-intersection allowed). lines [a,b] and [c,d] cross only if
// det(a,b,c)*det(a,b,d) <= 0 && det(c,d,a)*det(c,d,b) <= 0 (if they're = 0 then the intersection is in the end points 
// and we ignore this. Making the determinants (-1) or 1 to avoid overflow.
bool areIntersecting(GLPoint a, GLPoint b, GLPoint c, GLPoint d)
{
	int firstDeterminant = (a.x*b.y+a.y*c.x+b.x*c.y-c.x*b.y-c.y*a.x-b.x*a.y);
	int secondDeterminant = (a.x*b.y+a.y*d.x+b.x*d.y-d.x*b.y-d.y*a.x-b.x*a.y);
	int thirdDeterminant = (c.x*d.y+c.y*a.x+d.x*a.y-a.x*d.y-a.y*c.x-d.x*c.y);
	int fourthDeterminant = (c.x*d.y+c.y*b.x+d.x*b.y-b.x*d.y-b.y*c.x-d.x*c.y);
	firstDeterminant = (firstDeterminant > 0)?1:-1;
	secondDeterminant = (secondDeterminant > 0)?1:-1;
	thirdDeterminant = (thirdDeterminant > 0)?1:-1;
	fourthDeterminant = (fourthDeterminant > 0)?1:-1;
	return ( (firstDeterminant*secondDeterminant < 0) && (thirdDeterminant*fourthDeterminant < 0));
}
//checking if the point to be added will cause intersection(the line formed from the last added point and this line
//intersects some of the old lines?
bool isOKGLPoint(GLPoint a)
{
	if(polygon.size() < 3)
		return true;
	GLPoint last = polygon.at(polygon.size()-1);
	for(int j=0;j<polygon.size()-2;j++)
		if(areIntersecting(polygon.at(j),polygon.at(j+1),last,a))
			return false;
	return true;
}
//geting the centroid of the polygon 
GLPoint getCentroid()
{
	float x_sum = 0.f;
	float y_sum = 0.f;
	for(unsigned i = 0;i<polygon.size();i++)
	{
		x_sum += polygon[i].x;
		y_sum += polygon[i].y;
	}
	return GLPoint(x_sum/polygon.size(),y_sum/polygon.size());
}
//we want the centroid to be the start of our coordinate system, so we first translate each
//point with the (centroid.x,centroid.y) and afterwards apply rotating matrix. In the end we translate back
//each point to the original coordinate system used and return the rotated point
GLPoint getRotatedGLPoint(GLPoint centroid, GLPoint current, double angle,bool toRight)
{
	int x_coord;
	int y_coord;
	if(!toRight)
	{
		x_coord = (int) (current.x-centroid.x)*cos(angle) - (current.y-centroid.y)*sin(angle);
		y_coord = (int) (current.y-centroid.y)*cos(angle)+(current.x-centroid.x)*sin(angle);
	}
	else
	{
		x_coord = (int) (current.x - centroid.x)*cos(-angle) - (current.y - centroid.y)*sin(-angle);
		y_coord = (int) (current.y - centroid.y)*cos(-angle) + (current.x - centroid.x)*sin(-angle);
	}
	x_coord +=centroid.x;
	y_coord +=centroid.y;
	return GLPoint(x_coord,y_coord);
}
//rotating polygon = getting the rotated coordinates of each point, push it in the polygon(points) vector, empty
//the old vectors with edges and intersect points and redraw everything
void RotatePolygon(double angle, bool toRight)
{
	GLPoint centroid = getCentroid();
	vector<GLPoint> temp;
	int size = polygon.size();
	for(int i =0 ;i<size;i++)
	{
		polygon.at(i) = getRotatedGLPoint(centroid,polygon.at(i),angle,toRight);
	}
	ListOfEdges.erase(ListOfEdges.begin(),ListOfEdges.end());
	ListOfIntersectPoints.erase(ListOfIntersectPoints.begin(),ListOfIntersectPoints.end());
	flag_rotate = true;
	glutPostRedisplay();
}
//glut Init
void initView() {

	glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    gluOrtho2D(0.0, 640.0, 0.0, 480.0);

}
/* working with mouse events and checking if the points are ok to be added */
void mouse(int button, int state, int mx, int my)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if(isOKGLPoint(GLPoint(mx,my)) || current < 3)
		{
			polygon.push_back(GLPoint(mx,my));
			current++;
			glutPostRedisplay();
		}
	}
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{

		glFlush();
		int size = polygon.size();
		for(unsigned i = 0 ;i<size;i++)
			polygon.pop_back();
		current = 0;
		glutPostRedisplay();
	}
}
//working with keyboards events for filling and rotating the polygon
void keyboard(int key, int x, int y)
{
	if(key == GLUT_KEY_LEFT)
	{
		flag_rotate = true;
		flag = false;
		RotatePolygon(PI/6,false);
	}
	if(key == GLUT_KEY_RIGHT)
	{
		flag_rotate = true;
		flag = false;
		RotatePolygon(PI/6,true);
	}
	if(key == GLUT_KEY_UP)
	{
		if(isOKGLPoint(polygon.at(0)))
		{
			flag = true;
		}
		glutPostRedisplay();
	}

}
//drawing the points of the polygon
void drawGLPoints()
{
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	for(unsigned i =0 ;i<polygon.size();i++)
	{
		glColor3f(0.f,1.f,0.f);
		glVertex2i(polygon[i].x,polygon[i].y);
	}
	glEnd();
}
//drawing the edges of the polygon. flag is to show whether we're going to fill the polygon(add closing edge from the last to the fist point)
void drawEdges(bool flag)
{
	glColor3d(0,255,0);
	glBegin(GL_LINE_STRIP);
	int polygon_size = polygon.size();
	for(unsigned i = 0;i<polygon_size-1;i++)
	{
		glVertex2i(polygon[i].x,polygon[i].y);
		glVertex2i(polygon[i+1].x,polygon[i+1].y);
	}
	if(flag)
	{
		glVertex2i(polygon[polygon_size-1].x,polygon[polygon_size-1].y);
		glVertex2i(polygon[0].x,polygon[0].y);
	}
	glEnd();
}
void renderScene(void)
{
	glClearColor(0.f,0.f,0.f,0.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,640,480,0,-1,1);
	glViewport(0,0,640,480);
	//if we are going to rotate - clear all the buffers and redraw everything
	if(flag_rotate)
	{
		glFlush();
		drawGLPoints();
		drawEdges(true);
		initEdges();
		scanlineFill();
	}
	//flag shows whether we're going to fill the polygon or just add new points/adges
	if(!flag)
	{
		drawGLPoints();
		if(current>=2)
			drawEdges(flag);
	}
	if(flag)
	{	
		drawGLPoints();
		if(current>2)
		{
			drawEdges(flag);
			initEdges();
			scanlineFill();
		}
	}
	glFlush();
}
//when the user moves our window
void reshape(int w, int h) { 

	glViewport(0, 0, w, h); 

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	win_height = h;
	win_width = w;

}
//*************************** main >>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char** argv)
{
  glutInit(&argc, argv);          // initialize the toolkit
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // set display mode
  glutInitWindowSize(640,480);     // set window size
  glutInitWindowPosition(100, 150); // set window position on screen
  glutCreateWindow("ScanLineFillPolygon"); // open the screen window
  initView();//init parametrs for glut
  glutDisplayFunc(renderScene);
  glutSpecialFunc(keyboard);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);            
  glutMainLoop(); 		     // go into a perpetual loop
  return 0;
}
