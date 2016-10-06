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

	bool alive;
	int index;
};

void sizeOrdering(map<int, vector<PATCH>> src, vector<int> &dst, int startCol, int blanks, int counter) 
{
	if (counter == blanks)
	{
		return;
	}

	std::map<int, int> totalSize;
	for (int i = startCol; i < src.size(); i++)
	{
		int total = 0;

		for (int j = 0; j < counter; j++)
		{
			total += src[i - 1][src.size() - (counter - j)].dst.height;
		}

		for (int j = counter; j < src[i].size(); j++)
		{
			total += src[i][j].dst.height;
		}
		totalSize[i] = total;
	}

	std::vector<std::pair<int, int>>vec(totalSize.begin(), totalSize.end());
	std::sort(vec.begin(), vec.end(), compare_pair_second<std::greater>());

	vector<int> selete32;
	for (int i = 0; i < (blanks - counter) * 2; i++)
	{
		selete32.push_back(vec[i].first);
	}

	std::sort(selete32.begin(), selete32.end());

	
	for (int i = 0; i < (blanks - counter) * 2; i += 2)
	{
		dst.push_back(selete32[i]);
	}

	std::sort(dst.begin(), dst.end());
}

int main(void)
{
	int totalBlanks = 6;
	int wantBlanks = 5;
	int blackSize = 150;
	cv::Mat src = cv::imread("1.jpg", IMREAD_GRAYSCALE);
	cv::Mat dst = cv::imread("1.jpg");

	cv::threshold(src, src, 126, 255, CV_THRESH_BINARY);

	float mCols = 1.5f;


	int sizeR = 33;
	int sizeC = 28;
	int margin = 5;

	float colAddWeight = 0;
	float rowAddWeight = 0;

	vector<Rect> roi;

	int blank = 10000000;
	int width = src.cols / sizeC;
	int height = src.rows / sizeR;

	for (int i = 0; i < sizeR; i++)
	{
		for (int j = 0; j < sizeC; j++)
		{
			int x = width * j + (j * colAddWeight);
			int y = height * i + (i * rowAddWeight);

			int c = height;
			if (y + height > src.rows - 1)
			{
				c = src.rows - 1 - y;
			}

			Rect temp(x, y, width, c);

			//int top = y; 
			//int bottom = y + c;
			//int left = x;
			//int right = x + width;
			//int centerX = (left + right) / 2;
			//int centery = (top + bottom) / 2;

			int minX = blank;
			int minY = blank;
			int maxX = -blank;
			int maxY = -blank;

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
			int x1 = x + minX - margin;
			int x2 = x + maxX + margin;
			int y1 = y + minY - margin;
			int y2 = y + maxY + margin;

			if (x1 < 0)
			{
				x1 = 0;
			}

			if (y1 < 0)
			{
				y1 = 0;
			}

			if (x2 > src.cols - 1)
			{
				x2 = src.cols - 1;
			}

			if (y2 > src.rows - 1)
			{
				y2 = src.rows - 1;
			}

			Rect patchSize(cv::Point(x1, y1), cv::Point(x2, y2));

			if (minX == blank)
			{
				patchSize = Rect(x, y, blank, blank);
			}
			else
			{
				if (patch.at<cv::Vec3b>(0, 0)[1] != 255 ||
					patch.at<cv::Vec3b>(1, 1)[1] != 255 ||
					patch.at<cv::Vec3b>(2, 1)[1] != 255)
				{
					patchSize = Rect(x, y, blank, blank);
				}
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

	counter = 0;
	map<int, vector<PATCH>> orderedPatch;

	for (int i = 0; i < order.size(); i++)
	{
		int total = 0;
		for (int j = 0; j < order[i].size(); j++)
		{
			if (order[i][j].width == blank &&
				order[i][j].height == blank)
			{
				// 178 * 156
				//order[i][j].width = 178;
				//order[i][j].height = 156;
				order[i][j].width = blackSize;
				order[i][j].height = blackSize;

			}
			total += order[i][j].height;
		}

		total /= order[i].size();
		total /= 1.5f;

		for (int j = 0; j < order[i].size(); j++)
		{
			if (order[i][j].height < 50)
			{
				int top = order[i][j].y;
				int bottom = order[i][j].y + order[i][j].height;
				int center = (top + bottom) / 2;

				order[i][j].y = center - (total / 2);
				order[i][j].height = total;
			}
			
			PATCH tPatch;
			tPatch.src = order[i][j];
			tPatch.dst = order[i][j];
			tPatch.alive = true; 
			tPatch.index = counter++;

			orderedPatch[i].push_back(tPatch);
		}
	}

	counter = 0;
	vector<int> changedIdx;
	map<int, vector<PATCH>> reOrderData;
	for (int i = 0; i < orderedPatch.size(); i++)
	{
		vector<int> selete32;
		sizeOrdering(orderedPatch, selete32, i, wantBlanks, counter);
		int preHeight = 0;
		bool isBlank = false;
		for (int j = 0; j < counter; j++)
		{
			if (j == 0) // 첫 칸이 빈칸일 때
			{
				if (orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.height == blackSize &&
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.width == blackSize)
				{
					isBlank = true;
					continue;
				}
			}

			preHeight = orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.height * j;
			Rect tOrderRect = Rect(
				orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.x - (src.cols / sizeC),
				preHeight,
				orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.width,
				orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.height);

			PATCH t;
			t.src = orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].src;
			t.dst = tOrderRect;
			reOrderData[i].push_back(t);

			preHeight += orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.height;
		}

		vector<int>::iterator it = std::find(selete32.begin(), selete32.end(), i);
		if (it != selete32.end() && !isBlank) // 선택된것중 하나면서 빈칸이 아니면
		{
			for (int j = 0; j < orderedPatch[i].size() - counter; j++)
			{
				if (j == orderedPatch[i].size() - (counter + 1)) // 32번째 제거
				{
					continue;
				}

				int yH = orderedPatch[i][j].dst.height * j + preHeight;
				Rect tOrderRect = Rect(
					orderedPatch[i][j].dst.x,
					yH,
					orderedPatch[i][j].dst.width,
					orderedPatch[i][j].dst.height);

				PATCH t;
				t.src = orderedPatch[i][j].src;
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);
			}
			counter++;
			changedIdx.push_back(i);
		}
		else // 33칸
		{
			if (i == orderedPatch.size() - 1) // last line
			{
				for (int j = 0; j < orderedPatch[i].size() - counter - (totalBlanks - wantBlanks); j++)
				{
					int yH = orderedPatch[i][j].dst.height * j + preHeight;
					Rect tOrderRect = Rect(
						orderedPatch[i][j].dst.x,
						yH,
						orderedPatch[i][j].dst.width,
						orderedPatch[i][j].dst.height);

					PATCH t;
					t.src = orderedPatch[i][j].src;
					t.dst = tOrderRect;
					reOrderData[i].push_back(t);
				}
			}
			else
			{
				for (int j = 0; j < orderedPatch[i].size() - counter; j++)
				{
					int yH = orderedPatch[i][j].dst.height * j + preHeight;
					Rect tOrderRect = Rect(
						orderedPatch[i][j].dst.x,
						yH,
						orderedPatch[i][j].dst.width,
						orderedPatch[i][j].dst.height);

					PATCH t;
					t.src = orderedPatch[i][j].src;
					t.dst = tOrderRect;
					reOrderData[i].push_back(t);
				}
			}
		}
	}

	// 5650 * 4815
	float limit = 4800;
	counter = 0;

	cv::Mat target = cv::Mat::zeros(4815 + 350, src.cols * mCols + 300, dst.type());
	target = cv::Scalar(255, 255, 255);

	width = (src.cols * mCols) / sizeC;;
	int invers = reOrderData.size();
	for (int i = 0; i < reOrderData.size(); i++)
	{
		int total = 0;
		for (int j = 0; j < reOrderData[i].size(); j++)
		{
			total += reOrderData[i][j].dst.height;
		}

		invers--;
		indexer = 0;
		int differ = (limit - total) / ((float)reOrderData[i].size());

		int yH = differ;

		for (int j = 0; j < reOrderData[i].size(); j++)
		{
			int h = reOrderData[i][j].dst.height;

			// TODO 큰 글자 간엔		= 비교적 넓게 
			// 큰 - 작, 작 - 큰 은	= 중간
			// 작 - 작 은	=	좁게

			//if (h < 120)
			//{
			//	h += 15;
			//}
			//else
			//{
			//	h -= 15;
			//}

			reOrderData[i][j].dst.y = yH;
			yH += reOrderData[i][j].dst.height + differ;


			//reOrderData[i][j].dst.x = (reOrderData[i][j].dst.x * mCols) +(reOrderData[i][j].dst.width / 3);

			int left = reOrderData[i][j].dst.x;
			int right = reOrderData[i][j].dst.x + reOrderData[i][j].dst.width;
			int center = (left + right) / 2;

			int dif = center - left;
			int mar = width - dif;
			reOrderData[i][j].dst.x = ((width * invers) + mar);

			//width

			cv::Mat patch = dst(reOrderData[i][j].src);
			patch.copyTo(target(reOrderData[i][j].dst));
			cv::rectangle(target, reOrderData[i][j].dst, cv::Scalar(0, 0, 255), 1);
		}

		vector<int>::iterator it = std::find(changedIdx.begin(), changedIdx.end(), i);
		if (it != changedIdx.end())
		{
			Rect checkChanged = Rect(
				reOrderData[i][reOrderData[i].size() - 1].dst.x,
				reOrderData[i][reOrderData[i].size() - 1].dst.y + 300,
				reOrderData[i][reOrderData[i].size() - 1].dst.width,
				reOrderData[i][reOrderData[i].size() - 1].dst.height);
			cv::rectangle(target, checkChanged, cv::Scalar(0, 0, 255), 50);
			//cv::putText(target, to_string(indexer++), orderedPatch[i][j].dst.tl(), 1, 0.5, cv::Scalar(0, 255, 0));
		}
	}

	vector<int> compression_params;
	compression_params.push_back(IMWRITE_JPEG_QUALITY);
	compression_params.push_back(100);
	cv::imwrite("dst.jpg", target, compression_params);

	return 0;
}
