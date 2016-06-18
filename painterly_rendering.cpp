#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
#include <cmath>
#include <stdio.h>
#include <cstdlib>
#include <time.h>

using namespace std;
using namespace cv;

//manual
int fg = 1;//gridsize
int T = 5;//threshold
int minStrokeLength = 3;
int maxStrokeLength = 120;
float fc = 0.5;//0~1

////////////////////////////////////add class "Point" and line point!!//////////////////////////////

struct StrokePoint{
	int x;
	int y;
	StrokePoint* next;
};

class Stroke{
	public:
		StrokePoint* p;
		StrokePoint* p_end;
		int r;
		int colorB;
		int colorG;
		int colorR;
		Stroke* next;
		Stroke(int x, int y, int r, int colorB, int colorG, int colorR);
		Stroke();
};

Stroke::Stroke(int x, int y, int r, int colorB, int colorG, int colorR){
	this->p = new StrokePoint;
	this->p->x = x;
	this->p->y = y;
	this->p->next = NULL;
	this->p_end = this->p;
	this->r = r;
	this->colorB = colorB;
	this->colorG = colorG;
	this->colorR = colorR;
	this->next = NULL;
}

Stroke* S = NULL;
int lengthS = 0;

float diff(Vec3b a, Vec3b b){
	float d = sqrt(pow((float)a[0] - (float)b[0], 2)
					+ pow((float)a[1] - (float)b[1], 2)
					+ pow((float)a[2] - (float)b[2], 2));
	return d;
}

Stroke* makeStroke(int r, int x0, int y0, Mat referenceImage, Mat canvas){
	int strokeColorB = (int)referenceImage.at<Vec3b>(y0,x0)[0];
	int strokeColorG = (int)referenceImage.at<Vec3b>(y0,x0)[1];
	int strokeColorR = (int)referenceImage.at<Vec3b>(y0,x0)[2];
	Stroke* K = new Stroke(x0, y0, r, strokeColorB, strokeColorG, strokeColorR);

	int x = x0;
	int y = y0;
	float last_dx = 0;
	float last_dy = 0;

	Mat sobelx, sobely;
	Sobel(referenceImage, sobelx, -1, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(referenceImage, sobely, -1, 0, 1, 3, 1, 0, BORDER_DEFAULT);

	for(int i=0;i<maxStrokeLength;i++){
//		cout << "DDDDDDDDDD" << y << " " << x;
//		fflush(stdout);
		if(i>minStrokeLength && diff(referenceImage.at<Vec3b>(y,x), canvas.at<Vec3b>(y,x))<diff(referenceImage.at<Vec3b>(y,x), referenceImage.at<Vec3b>(y0,x0))){
//			cout << "AAAAAAAAAA";
//			fflush(stdout);
			return K;
		}
		if(sobelx.at<Vec3b>(y,x)[0]==0 && sobelx.at<Vec3b>(y,x)[1]==0 && sobelx.at<Vec3b>(y,x)[2]==0 && sobely.at<Vec3b>(y,x)[0]==0 && sobely.at<Vec3b>(y,x)[1]==0 && sobely.at<Vec3b>(y,x)[2]==0){
//			cout << "BBBBBBBBBB";
//			fflush(stdout);
			return K;
		}
//		cout << "CCCCCCCCCC";
//		fflush(stdout);
		float gx = (sobelx.at<Vec3b>(y,x)[0] + sobelx.at<Vec3b>(y,x)[1] + sobelx.at<Vec3b>(y,x)[2])/3;
		float gy = (sobely.at<Vec3b>(y,x)[0] + sobely.at<Vec3b>(y,x)[1] + sobely.at<Vec3b>(y,x)[2])/3;
		float dx = -gy;
		float dy = gx;
		if(last_dx * dx + last_dy * dy < 0){
			dx = -dx;
			dy = -dy;
		}

		dx = fc * dx + (1-fc) * last_dx;
		dy = fc * dy + (1-fc) * last_dy;
		float dxdyLength = sqrt(pow(dx, 2) + pow(dy, 2));
		dx = dx/dxdyLength;
		dy = dy/dxdyLength;

		x = x + r * dx;
		y = y + r * dy;
		last_dx = dx;
		last_dy = dy;
		if(x<0 || y<0 || x>=canvas.cols || y>=canvas.rows)
			break;
		else{
			StrokePoint *p_temp = new StrokePoint;
			p_temp->x = x;
			p_temp->y = y;
			p_temp->next = NULL;
			K->p_end->next = p_temp;
			K->p_end = K->p_end->next;
		}
//		cout << sobelx.cols << ", " << sobelx.rows << ", " << y << ", " << x << ", " << dx << ", " << dy << "\n";
//		fflush(stdout);
	}
	
	return K;
}

void paintLayer(Mat& canvas, Mat referenceImage, int r){
	cout << "start\n";
	fflush(stdout);
	float** difference = new float*[canvas.rows];
	for(int i=0;i<canvas.rows;i++)
		difference[i] = new float[canvas.cols];
	for(int i=0;i<canvas.cols;i++){
		for(int j=0;j<canvas.rows;j++){
			difference[j][i] = diff(canvas.at<Vec3b>(j,i), referenceImage.at<Vec3b>(j,i));
		}
	}
	int grid = fg * r;
	for(int i=0;i<canvas.cols;i+=grid){
		for(int j=0;j<canvas.rows;j+=grid){
			float areaError = 0;
			float pixelError = 0;
			int maxErrorX;
			int maxErrorY;
			for(int x=(i-grid)/2;x<=(i+grid)/2;x++){
				for(int y=(j-grid)/2;y<=(j+grid)/2;y++){
					if(x>=0 && y>=0 && i>=0 && j>=0 && i<=canvas.cols && j<=canvas.rows){
						areaError += difference[y][x] / pow(grid, 2);
					
						if(difference[y][x]>pixelError){
							maxErrorY = y;
							maxErrorX = x;
							pixelError = difference[y][x];
						}
					}
				}
			}

			if(areaError>T){
				Stroke* s = makeStroke(r, i, j, referenceImage, canvas);
				s->next = S;
				S = s;
				lengthS += 1;
			}
		}
	}

	srand(time(NULL));

	Stroke *pre, *cur;

	while(lengthS>0){
		int q = rand()%lengthS;
//		cout << lengthS << ", " << q << "\n";
//		fflush(stdout);
		pre=NULL; 
		cur = S;
		while(q!=0){
			pre = cur;
			cur = cur->next;
			q -= 1;
		}
		while(cur->p!=NULL){
			for(int i=cur->p->x-cur->r;i<=cur->p->x+cur->r;i++){
				for(int j=cur->p->y-cur->r;j<cur->p->y+cur->r;j++){
					if(i>=0 && j>=0 && i<=canvas.cols && j<=canvas.rows){
						if(pow(i-cur->p->x, 2) + pow(j-cur->p->y, 2)<=pow(cur->r, 2)){
							canvas.at<Vec3b>(j,i)[0] = cur->colorB;
							canvas.at<Vec3b>(j,i)[1] = cur->colorG;
							canvas.at<Vec3b>(j,i)[2] = cur->colorR;
						}
					}
				}
			}
			StrokePoint* tmp = cur->p;
			cur->p = cur->p->next;
			delete tmp;
		}
		if(pre!=NULL)
			pre->next = cur->next;
		else
			S = cur->next;
		delete cur;
		lengthS--;
	}

	for(int i=0;i<canvas.rows;i++)
		delete difference[i];
	delete difference;

	cout << "finished\n";
	fflush(stdout);
}

Mat paint(Mat sourceImage, int* R, int n){
	Mat canvas = sourceImage.clone();
	canvas.setTo(Scalar(255,255,255));

	for(int i=n-1;i>=0;i--){
		Mat referenceImage = canvas.clone();
		cout << i << "\n";
		fflush(stdout);
		GaussianBlur(sourceImage, referenceImage, Size(R[i]+1, R[i]+1), 0, 0);
		paintLayer(canvas, referenceImage, R[i]);
	}

	return canvas;
}



int main(int argc, const char *argv[])
{
	Mat sourceImage;
	sourceImage = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	stringstream ss;
	int n;
	ss << argv[3];
	ss >> n;
	int* brush = new int[n];
	for(int i=0;i<n;i++)
		brush[i] = pow(2, i+1);

	Mat canvas = paint(sourceImage, brush, n);
	delete brush;

	imwrite(argv[2], canvas);

//////////show in window/////////////
//	namedWindow("Origin", WINDOW_AUTOSIZE);
//	imshow("Origin", sourceImage);

//	namedWindow("Paint", WINDOW_AUTOSIZE);
//	imshow("Paint", canvas);

//	waitKey(0);

	return 0;
}

