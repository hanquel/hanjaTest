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

int main(void)
{
	cv::Mat src = cv::imread("1.jpg", IMREAD_GRAYSCALE);
	cv::Mat dst = cv::imread("1.jpg");

	cv::threshold(src, src, 127, 255, CV_THRESH_BINARY);

	float divide = 1;

	cv::resize(src, src, cv::Size(src.cols / divide, src.rows / divide));
	cv::resize(dst, dst, cv::Size(dst.cols / divide, dst.rows / divide));

	int sizeR = 33;
	int sizeC = 28;
	int margin = 2;

	float rowAddWeight = 0.26f;

	Rect temp;
	vector<Rect> roi;

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
			int x1 = x + minX - margin;
			int x2 = x + maxX + (margin * 2);
			int y1 = y + minY - margin;
			int y2 = y + maxY + (margin * 2);

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

	counter = 0;
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
			tPatch.alive = true;
			tPatch.index = counter++;

			orderedPatch[i].push_back(tPatch);
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
	cv::Mat target = cv::Mat::zeros(dst.rows, dst.cols, dst.type());
	target = cv::Scalar(255, 255, 255);
	map<int, vector<PATCH>> reOrderData;
	for (int i = 0; i < orderedPatch.size(); i++)
	{
		indexer = 0;
		vector<int>::iterator it = std::find(selete32.begin(), selete32.end(), i);
		if (it != selete32.end()) // 선택된것중 하나면
		{
			for (int j = 0; j < counter; j++)
			{
				int yH = orderedPatch[i - 1][0].dst.height * j;
				Rect tOrderRect = Rect(
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.x - (src.cols / sizeC),
					yH,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.width,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.height);

				//cv::Mat patch = dst(orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].src);
				//patch.copyTo(target(tOrderRect));

				PATCH t;
				t.src = orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].src;
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);

				//cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 70));
				//cv::rectangle(target, tOrderRect, cv::Scalar(0, 0, 255));
				//cv::imshow("target", target);
				//cv::imshow("dst", dst);
				//cv::waitKey();
			}

			for (int j = 0; j < orderedPatch[i].size() - counter; j++)
			{
				if (j == orderedPatch[i].size() - (counter + 1)) // 32번째 제거
				{
					continue;
				}

				int yH = orderedPatch[i][j].dst.height * j + (orderedPatch[i][0].dst.height * counter);
				Rect tOrderRect = Rect(
					orderedPatch[i][j].dst.x,
					yH,
					orderedPatch[i][j].dst.width,
					orderedPatch[i][j].dst.height);

				//cv::Mat patch = dst(orderedPatch[i][j].src);
				//patch.copyTo(target(tOrderRect));

				//reOrderData[i].push_back(tOrderRect);
				PATCH t;
				t.src = orderedPatch[i][j].src;
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);

				//cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 0));
				//cv::rectangle(target, tOrderRect, cv::Scalar(0, 0, 255));
				//cv::imshow("target", target);
				//cv::imshow("dst", dst);
				//cv::waitKey();
			}
			counter++;
		}
		else
		{
			for (int j = 0; j < counter; j++)
			{
				int yH = orderedPatch[i - 1][0].dst.height * j;
				Rect tOrderRect = Rect(
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.x - (src.cols / sizeC),
					yH,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.width,
					orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].dst.height);

				//cv::Mat patch = dst(orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].src);
				//patch.copyTo(target(tOrderRect));

				//reOrderData[i].push_back(tOrderRect);
				PATCH t;
				t.src = orderedPatch[i - 1][orderedPatch[i - 1].size() - (counter - j)].src;
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);

				//cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 0));
				//cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 0));
				//cv::imshow("target", target);
				//cv::imshow("dst", dst);
				//cv::waitKey();
			}
			
			for (int j = 0; j < orderedPatch[i].size() - counter; j++)
			{
				int yH = orderedPatch[i][j].dst.height * j + (orderedPatch[i - 1][0].dst.height * counter);
				Rect tOrderRect = Rect(
					orderedPatch[i][j].dst.x,
					yH,
					orderedPatch[i][j].dst.width,
					orderedPatch[i][j].dst.height);

				//cv::Mat patch = dst(orderedPatch[i][j].src);
				//patch.copyTo(target(tOrderRect));

				//reOrderData[i].push_back(tOrderRect);
				PATCH t;
				t.src = orderedPatch[i][j].src;
				t.dst = tOrderRect;
				reOrderData[i].push_back(t);

				//cv::putText(target, std::to_string(indexer++), tOrderRect.tl(), 1, 1, cv::Scalar(255, 220, 0));
				//cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 0));
				//cv::imshow("target", target);
				//cv::imshow("dst", dst);
				//cv::waitKey();
			}
		}
		//cv::imshow("target", target);
		//cv::imshow("dst", dst);
		//cv::waitKey();
	}


	int limit = 800;
	for (int i = 0; i < reOrderData.size(); i++)
	{
		int total = 0;
		for (int j = 0; j < reOrderData[i].size(); j++)
		{
			total += reOrderData[i][j].dst.height;
		}
		int differ = (limit - total) / (int)reOrderData[i].size();
		for (int j = 0; j < reOrderData[i].size(); j++)
		{
			int yHeight = reOrderData[i][j].dst.height * j;
			reOrderData[i][j].dst.y = yHeight + (differ * j);

			//cv::putText(target, to_string(orderedPatch[i][j].index), orderedPatch[i][j].dst.tl(), 1, 0.5, cv::Scalar(0, 255, 0));

			cv::Mat patch = dst(reOrderData[i][j].src);
			patch.copyTo(target(reOrderData[i][j].dst));
			//cv::rectangle(target, reOrderData[i][j].dst, cv::Scalar(0, 0, 255));

			//cv::imshow("target", target);
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
