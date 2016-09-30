#pragma once

#include "헤더.h"
#include <map>
#include <iostream>
#include <functional>
#include "Patch.h"

#define TEST 0

using namespace std;
using namespace cv;

template<template<typename>class P = std::greater> struct compare_pair_second {
	template<class T1, class T2> bool operator()(const std::pair<T1, T2>&left, const std::pair<T1, T2>&right) {
		return P<T2>()(left.second, right.second);
	}
};

Rect findCharSizeFromROI(cv::Mat patch, cv::Rect rect, int x, int y, int cols, int rows, int margin);

void getSplitedROI(vector<Rect> &roi, cv::Mat img, int sizeR, int sizeC, int margin, int rowWeight, int colWeight);

void setRowBasedOrder(map<int, vector<Rect>> &rowBased, vector<Rect> roi, int rows, int cols)
{

}

void checkBoundary(int &x1, int &x2, int &y1, int &y2, int cols, int rows);

// 5645*4815
int main(void)
{
	cv::Mat src = cv::imread("1.jpg", IMREAD_GRAYSCALE);
	cv::Mat dst = cv::imread("1.jpg");

	cv::threshold(src, src, 127, 255, CV_THRESH_BINARY);

	//float divide = 1;

	//cv::resize(src, src, cv::Size(src.cols / divide, src.rows / divide));
	//cv::resize(dst, dst, cv::Size(dst.cols / divide, dst.rows / divide));

	cv::Mat target = cv::Mat::zeros(4815, 5645, dst.type());
	target = cv::Scalar(255, 255, 255);

	int sizeR = 33;
	int sizeC = 28;
	int margin = 10;

	float rowWeight = 0.26f;
	float colWeight = 0.0f;

	vector<Rect> roi;
	Rect standard;
	//getSplitedROI(roi, src, sizeR, sizeC, margin, rowWeight, colWeight);

	for (int i = 0; i < sizeR; i++)
	{
		for (int j = 0; j < sizeC; j++)
		{
			int x = src.cols / sizeC * j + (j * colWeight);
			int y = src.rows / sizeR * i + (i * rowWeight);
			standard = Rect(x, y, src.cols / sizeC, src.rows / sizeR);

			cv::Mat patch = src(standard);
			int minX = 10000000;
			int minY = 10000000;
			int maxX = -10000000;
			int maxY = -10000000;

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

			checkBoundary(x1, x2, y1, y2, sizeC, sizeR);

			Rect patchSize(cv::Point(x1, y1), cv::Point(x2, y2));

			if (minX == 1000000)
			{
				patchSize = standard;
			}

			//Rect t = Rect(findCharSizeFromROI(src(standard), standard, x, y, src.cols, src.rows, margin));
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

#if 0
			if (src(roi[index]).at<unsigned char>(0, 0) == 0)
			{
				continue;
			}
#endif

			order[counter].push_back(roi[index]);

			indexer++;
		}
	}

	counter = 0;
	std::map<int, int> totalSize;
	map<int, vector<Rect>> orderedPatch;

	for (int i = 0; i < order.size(); i++)
	{
		int blank = 0;
		int total = 0;
		for (int j = 0; j < order[i].size(); j++)
		{
			//if (order[i][j].width == (src.cols / sizeC) &&
			//	order[i][j].height == (src.rows / sizeR))
			//{
			//	blank++;
			//	continue;
			//}
			total += order[i][j].height;
		}
		totalSize[i] = total;
		total /= (order[i].size() /*- blank*/);

		for (int j = 0; j < order[i].size(); j++)
		{
			int top = order[i][j].y;
			int bottom = order[i][j].y + order[i][j].height;
			int center = (top + bottom) / 2;

			order[i][j].y = center - (total / 2);
			order[i][j].height = total;
			
			orderedPatch[i].push_back(order[i][j]);
		}
	}

	std::vector<std::pair<int, int>>vec(totalSize.begin(), totalSize.end());
	std::sort(vec.begin(), vec.end(), compare_pair_second<std::greater>());

	vector<int> tselete32;
	for (int i = 0; i < 12; i++)
	{
		tselete32.push_back(vec[i].first);
	}

	std::sort(tselete32.begin(), tselete32.end());

	vector<int> selete32;
	for (int i = 0; i < 12; i += 2)
	{
		selete32.push_back(tselete32[i]);
	}

	std::sort(selete32.begin(), selete32.end());

	counter = 0;

	map<int, vector<PATCH>> reOrderData;
	for (int i = 0; i < orderedPatch.size(); i++)
	{
		indexer = 0;
		vector<int>::iterator it = std::find(selete32.begin(), selete32.end(), i);
		if (it != selete32.end()) // 선택된것중 하나면
		{
			for (int j = 0; j < counter; j++)
			{
				int yH = orderedPatch[i - 1][0].height * j;
				Rect tOrderRect = Rect(
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].x - (src.cols / sizeC),
					yH,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].width,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].height);

#if TEST
				cv::Mat patch = dst(orderdPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].src);
				patch.copyTo(target(tOrderRect));
				cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 70));
				cv::rectangle(target, tOrderRect, cv::Scalar(0, 0, 255));
				cv::imshow("target", target);
				cv::imshow("dst", dst);
				cv::waitKey();
#endif

				PATCH t;
				t.src = orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)];
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);
			}

			for (int j = 0; j < orderedPatch[i].size() - counter; j++)
			{
				if (j == orderedPatch[i].size() - (counter + 1)) // 32번째 제거
				{
					continue;
				}

				int yH = orderedPatch[i][j].height * j + (orderedPatch[i][0].height * counter);
				Rect tOrderRect = Rect(
					orderedPatch[i][j].x,
					yH,
					orderedPatch[i][j].width,
					orderedPatch[i][j].height);

#if TEST
				cv::Mat patch = dst(orderedPatch[i][j].src);
				patch.copyTo(target(tOrderRect));
				cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 0));
				cv::rectangle(target, tOrderRect, cv::Scalar(0, 0, 255));
				cv::imshow("target", target);
				cv::imshow("dst", dst);
				cv::waitKey();
#endif

				PATCH t;
				t.src = orderedPatch[i][j];
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);
			}
			counter++;
		}
		else
		{
			for (int j = 0; j < counter; j++)
			{
				int yH = orderedPatch[i - 1][0].height * j;
				Rect tOrderRect = Rect(
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].x - (src.cols / sizeC),
					yH,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].width,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].height);

#if TEST
				cv::Mat patch = dst(orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].src);
				patch.copyTo(target(tOrderRect));
				cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 0));
				cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 0));
				cv::imshow("target", target);
				cv::imshow("dst", dst);
				cv::waitKey();
#endif

				PATCH t;
				t.src = orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)];
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);
			}
			
			for (int j = 0; j < orderedPatch[i].size() - counter; j++)
			{
				int yH = orderedPatch[i][j].height * j + (orderedPatch[i - 1][0].height * counter);
				Rect tOrderRect = Rect(
					orderedPatch[i][j].x,
					yH,
					orderedPatch[i][j].width,
					orderedPatch[i][j].height);

#if TEST
				cv::Mat patch = dst(orderedPatch[i][j].src);
				patch.copyTo(target(tOrderRect));

				cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 0));
				cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 0));
				cv::imshow("target", target);
				cv::imshow("dst", dst);
				cv::waitKey();
#endif

				PATCH t;
				t.src = orderedPatch[i][j];
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);
			}
		}
	}


	int limit = 1000;
	for (int i = 0; i < reOrderData.size(); i++)
	{
		int total = 0;
		for (int j = 0; j < reOrderData[i].size(); j++)
		{
			total += reOrderData[i][j].dst.height;
		}

		indexer = -1;
		int differ = (limit - total) / (int)reOrderData[i].size();
		for (int j = 0; j < reOrderData[i].size(); j++)
		{
			indexer++;
			int yHeight = reOrderData[i][j].dst.height * j;
			reOrderData[i][j].dst.y = yHeight + (differ * j);
			//reOrderData[i][j].dst.height = (reOrderData[i][j].dst.y + reOrderData[i][j].dst.height)- (differ * j);

			if (reOrderData[i][j].dst.y < 0)
			{
				reOrderData[i][j].dst.y = 0;
			}

			if (reOrderData[i][j].dst.x < 0)
			{
				reOrderData[i][j].dst.x = 0;
			}

#if 1
			if (reOrderData[i][j].dst.x + reOrderData[i][j].dst.width > src.cols - 1)
			{
				int differW = (src.cols - 1) - (reOrderData[i][j].dst.x + reOrderData[i][j].dst.width);
				reOrderData[i][j].src.width += differW;
				reOrderData[i][j].dst.width += differW;
			}

			if (reOrderData[i][j].dst.y + reOrderData[i][j].dst.height > src.rows - 1)
			{
				int differH = (src.rows - 1) - (reOrderData[i][j].dst.y + reOrderData[i][j].dst.height);
				reOrderData[i][j].src.height += differH;
				reOrderData[i][j].dst.height += differH;
			}
#endif

			if (reOrderData[i][j].src.y < 0)
			{
				reOrderData[i][j].src.y = 0;
			}

			if (reOrderData[i][j].src.x < 0)
			{
				reOrderData[i][j].src.x = 0;
			}

			if (reOrderData[i][j].src.x + reOrderData[i][j].src.width > src.cols - 1)
			{
				int differW = (src.cols - 1) - (reOrderData[i][j].src.x + reOrderData[i][j].src.width);
				reOrderData[i][j].src.width += differW;
				reOrderData[i][j].dst.width += differW;
			}

			if (reOrderData[i][j].src.y + reOrderData[i][j].src.height > src.rows - 1)
			{
				int differH = (src.rows - 1) - (reOrderData[i][j].src.y + reOrderData[i][j].src.height);
				reOrderData[i][j].src.height += differH;
				reOrderData[i][j].dst.height += differH;
			}

			cv::Mat patch;
			try
			{
				patch = dst(reOrderData[i][j].src);
			}
			catch (Exception e)
			{
				cout << e.err << endl;
			}

			Rect tempS = reOrderData[i][j].src;
			Rect tempD = reOrderData[i][j].dst;

			patch.copyTo(target(reOrderData[i][j].dst));
			cv::rectangle(target, reOrderData[i][j].dst, cv::Scalar(0, 0, 255));
			//cv::putText(target, std::to_string(indexer), reOrderData[i][j].dst.tl(), 1, 1, cv::Scalar(255, 0, 0));
			//cv::imshow("target", target);
			//cv::imshow("Src", dst);
			//cv::waitKey();

		}
	}

 	cv::imshow("target", target);
	cv::imshow("Src", dst);
	cv::waitKey();

	vector<int> compression_params;
	compression_params.push_back(IMWRITE_JPEG_QUALITY);
	compression_params.push_back(100);
	cv::imwrite("dst.jpg", target, compression_params);

	return 0;
}

Rect findCharSizeFromROI(cv::Mat patch, cv::Rect rect, int x, int y, int cols, int rows, int margin)
{
	int minX = 10000000;
	int minY = 10000000;
	int maxX = -10000000;
	int maxY = -10000000;

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

	checkBoundary(x1, x2, y1, y2, cols, rows);

	Rect patchSize(cv::Point(x1, y1), cv::Point(x2, y2));

	if (minX == 1000000)
	{
		patchSize = rect;
	}

	return patchSize;
}

void getSplitedROI(vector<Rect> &roi, cv::Mat img, int sizeR, int sizeC, int margin, int rowWeight, int colWeight) // 왜 안됨?
{
	for (int i = 0; i < sizeR; i++)
	{
		for (int j = 0; j < sizeC; j++)
		{
			int x = img.cols / sizeC * j + (j * colWeight);
			int y = img.rows / sizeR * i + (i * rowWeight);
			Rect temp(x, y, img.cols / sizeC, img.rows / sizeR);
			roi.push_back(findCharSizeFromROI(img(temp), temp, x, y, img.cols, img.rows, margin));
		}
	}
}

void checkBoundary(int &x1, int &x2, int &y1, int &y2, int cols, int rows)
{
	if (x1 < 0)
	{
		x1 = 0;
	}

	if (y1 < 0)
	{
		y1 = 0;
	}

	if (x2 > cols - 1)
	{
		x2 = cols - 1;
	}

	if (y2 > rows - 1)
	{
		y2 = rows - 1;
	}
}