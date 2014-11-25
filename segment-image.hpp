/*
Copyright (C) 2014 Adrian Yuen
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef _SEGMENT_IMAGE_HPP_
#define _SEGMENT_IMAGE_HPP_

//Debug
#include <iostream>

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "misc.hpp"
#include "segment-graph.hpp"

using namespace cv;

static inline float diff(Mat &b, Mat &g, Mat &r,
			 int x1, int y1, int x2, int y2);

static Vec3b randomColor( RNG& rng )
{
  int icolor = (unsigned) rng;
  return Vec3b( icolor&255, (icolor>>8)&255, (icolor>>16)&255 );
}

// dissimilarity measure between pixels
static inline float diff(vector<Mat> &src,
			 int x1, int y1, int x2, int y2) {

	float sum = 0;
	for (int ch = 0; ch < src.size(); ch++) {
		sum += square(src[ch].at<uchar>(y1,x1) - src[ch].at<uchar>(y2,x2));
	}
	return sqrt(sum);
}

/*
 * Segment an image
 *
 * Returns a color image representing the segmentation.
 *
 * im: image to segment.
 * sigma: to smooth the image.
 * k: constant for threshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation.
 */
void segment_image(Mat &im, float sigma, float k, universe * &u, vector<edge> &edges) {

	int rows = im.rows;
	int cols = im.cols;

  // smooth each color channel
	Mat matSmooth;
  GaussianBlur(im, matSmooth, Size(3,3), sigma);
 	vector<Mat> vSplit;
 	split(matSmooth, vSplit);

  // build graph
 	if (edges.size() != 0)
 		edges.clear();
  //for (int y = 0; y < height; y++) {
  for (int r = 0; r < rows; r++) {
  	//for (int x = 0; x < width; x++) {
  	for (int c = 0; c < cols; c++) {

      if (c < cols-1) {
      	edge new_edge;
      	new_edge.a = r * cols + c;
      	new_edge.b = r * cols + (c+1);
      	new_edge.w = diff(vSplit, c, r, c+1, 1);
      	edges.push_back(new_edge);
      }

      if (r < rows-1) {
      	edge new_edge;
      	new_edge.a = r * cols + c;
      	new_edge.b = (r+1) * cols + c;
      	new_edge.w = diff(vSplit, c, r, c, r+1);
      	edges.push_back(new_edge);
      }

      if ((c < cols-1) && (r < rows-1)) {
      	edge new_edge;
      	new_edge.a = r * cols + c;
      	new_edge.b = (r+1) * cols + (c+1);
      	new_edge.w = diff(vSplit, c, r, c+1, r+1);
      	edges.push_back(new_edge);
      }

      if ((c < cols-1) && (r > 0)) {
      	edge new_edge;
      	new_edge.a = r * cols + c;
      	new_edge.b = (r-1) * cols + (c+1);
      	new_edge.w = diff(vSplit, c, r, c+1, r-1);
      	edges.push_back(new_edge);
      }
    }
  } // end buld graph
  // segment
  u = segment_graph(cols*rows, edges.size(), &(edges[0]), k);
}

void post_process_image (universe *u, vector<edge> edges, int min_size, int *num_ccs = NULL) {
// post process small component
	for (int i = 0; i < edges.size(); i++) {
		int a = u->find(edges[i].a);
		int b = u->find(edges[i].b);
		if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
			u->join(a, b);
	}
	if (num_ccs != NULL)
		*num_ccs = u->num_sets();
}

Mat create_segmented_image (int rows, int cols, universe *u) {
	Mat output (rows, cols, CV_8UC3);

	// pick random colors for each component
	// NOTE: not actually random; segment_image is colored with
	// the same sequence...
	RNG rngColor (0xFFFFFFFF);
	Vec3b colors [cols*rows];
	for (int i = 0; i < cols*rows; i++) {
		colors[i] = randomColor(rngColor);
	}

	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			int comp = u->find(r * cols + c);
			output.at<Vec3b>(r,c) = colors[comp];
		}
	}
	return output;
}


#endif
