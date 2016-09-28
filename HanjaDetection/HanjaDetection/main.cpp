#include <map>
#include <iostream>
#include <functional>
#include <opencv2/core/mat.hpp>  
#include <opencv2/imgcodecs.hpp>  
#include <opencv2/imgproc.hpp> 
#include <opencv2/highgui.hpp>  

using namespace std;
using namespace cv;

template<template<typename>class P = std::greater> struct compare_pair_second {
	template<class T1, class T2> bool operator()(const std::pair<T1, T2>&left, const std::pair<T1, T2>&right) {
		return P<T2>()(left.second, right.second);
	}
};

class PATCH
{
public:
	Rect src;
	Rect dst;
};


//void seletedRowReordering()
//{
//	//4. 정렬
//	cv::Mat targetImg = cv::Mat::zeros(cv::Size(src.cols, src.rows), dst.type());
//	map<int, vector<Rect>> copyRoi;
//	for (int j = /*sortedRoi.size() - 5*/0; j < sortedRoi.size(); j++)
//	{
//		int yH = 0;
//		for (int i = 0; i < sortedRoi[j].size(); i++)
//		{
//			cv::Mat patch = dst(Rect(sortedRoi[j][i].x, sortedRoi[j][i].y, sortedRoi[j][i].width, sortedRoi[j][i].height));
//			
//			if (j == sortedRoi.size() - 5)
//			{
//				if (i < 1)
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x - (src.cols / sizeC), yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//				else
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x, yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//			}
//			else if (j == sortedRoi.size() - 4)
//			{
//				if (i < 2)
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x - (src.cols / sizeC), yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//				else
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x, yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//			}
//			else if (j == sortedRoi.size() - 3)
//			{
//				if (i < 3)
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x - (src.cols / sizeC), yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//				else
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x, yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//			}
//			else if (j == sortedRoi.size() - 2)
//			{
//				if (i < 4)
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x - (src.cols / sizeC), yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//				else
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x, yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//			}
//			else if (j == sortedRoi.size() - 1)
//			{
//				if (i < 5)
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x - (src.cols / sizeC), yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//				else
//				{
//					patch.copyTo(targetImg(Rect(sortedRoi[j][i].x, yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//				}
//			}
//			else
//			{
//				patch.copyTo(targetImg(Rect(sortedRoi[j][i].x, yH, sortedRoi[j][i].width, sortedRoi[j][i].height)));
//			}
//			
//			yH += (src.rows / sizeR);//sortedRoi[j][i].height;
//			//cv::imshow("Src", targetImg);
//			//cv::waitKey();
//		}
//	}
//}

int main(void)
{
	cv::Mat src = cv::imread("1.jpg", IMREAD_GRAYSCALE);
	cv::Mat dst = cv::imread("1.jpg");

	cv::threshold(src, src, 127, 255, CV_THRESH_BINARY);

	float divide = 7;

	cv::resize(src, src, cv::Size(src.cols / divide, src.rows / divide));
	cv::resize(dst, dst, cv::Size(dst.cols / divide, dst.rows / divide));

	int sizeR = 33;
	int sizeC = 28;
	int margin = 2;

	float rowAddWeight = 0.26f;

	Rect temp;
	vector<Rect> roi;

#if 0
	for (int i = 0; i < sizeR; i++)
	{
		cv::line(src, cv::Point(0, src.rows / sizeR * i + (i * rowAddWeight)), cv::Point(src.cols, src.rows / sizeR * i + (i * rowAddWeight)), cv::Scalar(255, 0, 0), 1);
	}

	for (int i = 0; i < sizeC; i++)
	{
		cv::line(src, cv::Point(src.cols / sizeC * i, 0), cv::Point(src.cols / sizeC * i, src.rows), cv::Scalar(255, 0, 0), 1);
	}

#endif

	for (int i = 0; i < sizeR; i++)
	{
		for (int j = 0; j < sizeC; j++)
		{
			int x = src.cols / sizeC * j;
			int y = src.rows / sizeR * i + (i * rowAddWeight);
			Rect temp(x, y, src.cols / sizeC, src.rows / sizeR);

			int minX = 10000;
			int minY = 10000;
			int maxX = -10000;
			int maxY = -10000;

			cv::Mat patch = src(temp);

			for (int m = 0; m < patch.rows; m++)
			{
				for (int n = 0; n < patch.cols; n++)
				{
					int value = patch.at<unsigned char>(m, n);
					if (value != 255)
					{
						if (maxY < m)
						{
							maxY = m;
						}

						if (maxX < n)
						{
							maxX = n;
						}

						if (minY > m)
						{
							minY = m;
						}

						if (minX > n)
						{
							minX = n;
						}
					}
				}
			}

			Rect patchSize(cv::Point(x + minX - margin, y + minY - margin), cv::Point(x + maxX + (margin * 2), y + maxY + (margin * 2)));

			if (minX == 10000)
			{
				patchSize = temp;
			}

			roi.push_back(patchSize);
		}
	}

	int counter = 0;
	int indexer = 0;
	map<int, vector<Rect>> order;

	for (int i = sizeC - 1; i > -1; i--)
	{
		for (int j = 0; j < sizeR; j++)
		{
			int index = i + j * sizeC;

			if (indexer >= sizeR && indexer % sizeR == 0)
			{
				counter++;
			}

			order[counter].push_back(roi[index]);

			indexer++;
		}
	}

	std::map<int, int> totalSize;
	map<int, vector<PATCH>> orderedPatch;

	for (int i = 0; i < order.size(); i++)
	{
		int blank = 0;
		int total = 0;
		for (int j = 0; j < order[i].size(); j++)
		{
			if (order[i][j].width == (src.cols / sizeC) &&
				order[i][j].height == (src.rows / sizeR))
			{
				blank++;
				continue;
			}
			total += order[i][j].height;
		}
		totalSize[i] = total;
		total /= (order[i].size() - blank);

		for (int j = 0; j < order[i].size(); j++)
		{
			int top = order[i][j].y;
			int bottom = order[i][j].y + order[i][j].height;
			int center = (top + bottom) / 2;

			order[i][j].y = center - (total / 2);
			order[i][j].height = total;

			PATCH tPatch;
			tPatch.src = order[i][j];
			tPatch.dst = order[i][j];

			orderedPatch[i].push_back(tPatch);
			/*cv::rectangle(dst, orderedPatch[i][j].src, cv::Scalar(0, 0, 255));*/
		}
	}

	std::vector<std::pair<int, int>>vec(totalSize.begin(), totalSize.end());
	std::sort(vec.begin(), vec.end(), compare_pair_second<std::greater>());

	vector<int> selete32;
	for (int i = 0; i < 6; i++)
	{
		selete32.push_back(vec[i].first);
	}

	for (int i = 0; i < orderedPatch.size(); i++)
	{
		vector<int>::iterator it = std::find(selete32.begin(), selete32.end(), i);
		if (it != selete32.end())
		{
			for (int j = 0; j < orderedPatch[i].size(); j++)
			{
				if (j % (sizeR - 1) == 0 && j != 0)
				{
					for (int p = 0; p < orderedPatch.size(); p++)
					{
						for (int m = 0; m < orderedPatch[p].size(); m++)
						{
							int yH = (orderedPatch[p][m].dst.height * m);
							orderedPatch[p][m].dst.x = orderedPatch[p][m].dst.x - (src.cols / sizeC);
							orderedPatch[p][m].dst.y = yH;
							orderedPatch[p][m].dst.width = orderedPatch[p][m].dst.width;
							orderedPatch[p][m].dst.height = orderedPatch[p][m].dst.height;
						}
					}
				}
			}
		}
	}

	int limit = 1000;
	for (int i = 0; i < orderedPatch.size(); i++)
	{
		int si = orderedPatch[i][0].src.height * orderedPatch[i].size();
		int differ = (limit - si) / (int)orderedPatch[i].size();
		for (int j = 0; j < orderedPatch[i].size(); j++)
		{
			int yHeight = orderedPatch[i][j].src.height * j;
			orderedPatch[i][j].src.y = yHeight + (differ * j);
			orderedPatch[i][j].src.height = orderedPatch[i][j].src.height;

			cv::rectangle(dst, orderedPatch[i][j].src, cv::Scalar(0, 0, 255));
			//cv::imshow("Src", dst);
			//cv::waitKey();
		}
	}


	cv::imshow("Src", dst);
	cv::waitKey();

#if _NDEBUG
	vector<int> compression_params;
	compression_params.push_back(IMWRITE_JPEG_QUALITY);
	compression_params.push_back(100);
	cv::imwrite("dst.jpg", dst, compression_params);
#endif
	return 0;
}
