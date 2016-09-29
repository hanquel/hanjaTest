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

	float divide = 7;

	cv::resize(src, src, cv::Size(src.cols / divide, src.rows / divide));
	cv::resize(dst, dst, cv::Size(dst.cols / divide, dst.rows / divide));

	int sizeR = 33;
	int sizeC = 28;
	//int margin = 2;
	int margin = 0;

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
			int x1 = x + minX - margin;
			int x2 = x + maxX + (margin * 2);
			int y1 = y + minY - margin;
			int y2 = y + maxY + (margin * 2);

			//if (x1 < 0)
			//{
			//	x1 = 0;
			//}

			//if (y1 < 0)
			//{
			//	y1 = 0;
			//}


			//if (x2 > patch.rows - 1)
			//{
			//	x2 = patch.rows - 1;
			//}

			//if (y2 > patch.cols - 1)
			//{
			//	y2 = patch.cols - 1;
			//}

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
	//map<int, vector<Rect>> reOrderData;
	for (int i = 0; i < orderedPatch.size(); i++)
	{
		vector<int>::iterator it = std::find(selete32.begin(), selete32.end(), i);
		if (it != selete32.end()) // 선택된것중 하나면
		{
#if 0
			for (int j = 0; j < orderedPatch[i].size(); j++) // 32 번째로 정렬
			{				
				if (j % (sizeR - 1) == 0 && j != 0) // 33 번째 문자일 떄
				{
					for (int p = i + 1; p < orderedPatch.size(); p++) // 다음칸부터
					{
						Rect tOrderRect = Rect(	// 현재 마지막 칸을 다음 첫번쨰로 옮김
							orderedPatch[p - 1][orderedPatch[p].size() - 1].dst.x - (src.cols / sizeC),
							0,
							orderedPatch[p - 1][orderedPatch[p].size() - 1].dst.width,
							orderedPatch[p - 1][orderedPatch[p].size() - 1].dst.height);
						//reOrderData[p].push_back(tOrderRect);
						//orderedPatch[p - 1][orderedPatch[p].size() - 1].dst = tOrderRect;

						//cv::Mat patch = dst(orderedPatch[p - 1][orderedPatch[p].size() - 1].src);
						//patch.copyTo(target(tOrderRect));
						//cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 255));
						//cv::imshow("target", target);
						//cv::imshow("dst", dst);
						//cv::waitKey();

						for (int m = 1; m < orderedPatch[p].size(); m++) // 다음줄부터
						{
							int yH = (orderedPatch[p][m].dst.height * (m - 1));
							Rect tOrderRect = Rect(	// 첫번쨰 놈을 한칸씩 미룸
								orderedPatch[p][m - 1].dst.x,
								orderedPatch[p - 1][orderedPatch[p].size() - 1].dst.height + yH,
								orderedPatch[p][m - 1].dst.width,
								orderedPatch[p][m - 1].dst.height);
							//reOrderData[p].push_back(tOrderRect);

							//orderedPatch[p][m - 1].dst = tOrderRect;

							//patch = dst(orderedPatch[p][m - 1].src);
							//patch.copyTo(target(tOrderRect));

							//cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 255));
							//cv::imshow("target", target);
							//cv::imshow("dst", dst);
							//cv::waitKey();
						}
					}
				}
				else
				{
					int yH = orderedPatch[i][j].dst.height * j;
					Rect tOrderRect = Rect(	
						orderedPatch[i][j].dst.x,
						yH,
						orderedPatch[i][j].dst.width,
						orderedPatch[i][j].dst.height);

					//orderedPatch[i][j].dst = tOrderRect;
					//reOrderData[i].push_back(tOrderRectt);

					//cv::Mat patch = dst(orderedPatch[i][j].src);
					//patch.copyTo(target(tOrderRect));

					//cv::rectangle(target, tOrderRect, cv::Scalar(26, 0, 255));
					//cv::imshow("target", target);
					//cv::imshow("dst", dst);
					//cv::waitKey();
				}
			}
#endif
			counter++;
		}
		else
		{
			//for (int j = 0; j < counter; j++)
			//{
			//	Rect tOrderRect = Rect(
			//		orderedPatch[i - 1][orderedPatch[i - 1].size() - counter].dst.x,
			//		orderedPatch[i - 1][0].dst.height * (counter - 1),
			//		orderedPatch[i - 1][orderedPatch[i - 1].size() - counter].dst.width,
			//		orderedPatch[i - 1][orderedPatch[i - 1].size() - counter].dst.height);

			//	cv::Mat patch = dst(orderedPatch[i][j].src);
			//	patch.copyTo(target(tOrderRect));

			//	cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 0));
			//	cv::imshow("target", target);
			//	cv::imshow("dst", dst);
			//	cv::waitKey();
			//}
			//
			for (int j = 0; j < orderedPatch[i].size() - counter; j++)
			{
				int yH = orderedPatch[i][j].dst.height * j + (orderedPatch[i - 1][0].dst.height * counter);
				Rect tOrderRect = Rect(
					orderedPatch[i][j].dst.x,
					yH,
					orderedPatch[i][j].dst.width,
					orderedPatch[i][j].dst.height);

				//orderedPatch[i][j].dst = tOrderRect;
				//reOrderData[i].push_back(tOrderRect);

				cv::Mat patch = dst(orderedPatch[i][j].src);
				patch.copyTo(target(tOrderRect));

				cv::rectangle(target, tOrderRect, cv::Scalar(255, 0, 0));
			}
		}
		cv::imshow("target", target);
		cv::imshow("dst", dst);
		cv::waitKey();
	}


	//int limit = 650;
	//for (int i = 0; i < orderedPatch.size(); i++)
	//{
	//	int si = orderedPatch[i][0].src.height * orderedPatch[i].size();
	//	int differ = (limit - si) / (int)orderedPatch[i].size();
	//	for (int j = 0; j < orderedPatch[i].size(); j++)
	//	{
	//		if (orderedPatch[i][j].alive)
	//		{
	//			int yHeight = orderedPatch[i][j].src.height * j;
	//			orderedPatch[i][j].src.y = yHeight + (differ * j);
	//			orderedPatch[i][j].src.height = orderedPatch[i][j].src.height;

	//			//cv::putText(target, to_string(orderedPatch[i][j].index), orderedPatch[i][j].dst.tl(), 1, 0.5, cv::Scalar(0, 255, 0));
	//			cv::rectangle(target, orderedPatch[i][j].src, cv::Scalar(0, 0, 255));
	//		}

	//		//cv::imshow("target", target);
	//		//cv::waitKey();
	//	}
	//}


 	cv::imshow("target", target);
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
