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

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "misc.hpp"
#include "segment-graph.hpp"

using namespace cv;

static Vec3b randomColor( RNG& rng )
{
  int icolor = (unsigned) rng;
  return Vec3b( icolor&255, (icolor>>8)&255, (icolor>>16)&255 );
}

// dissimilarity measure between pixels
static inline float diff(Mat &b, Mat &g, Mat &r,
			 int x1, int y1, int x2, int y2) {

	return sqrt( 	square( b.at<uchar>(y1,x1) - b.at<uchar>(y2,x2) ) +
								square( g.at<uchar>(y1,x1) - g.at<uchar>(y2,x2) ) +
								square( r.at<uchar>(y1,x1) - r.at<uchar>(y2,x2) ) );
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
Mat segment_image(Mat *im, float sigma, float k, int min_size, int *num_ccs) {
  //int width = im->width();
  //int height = im->height();
	int rows = im->rows;
	int cols = im->cols;

  // smooth each color channel
	vector<Mat> vMatImage;
  split(*im, vMatImage);
  //image<float> *smooth_r = smooth(r, sigma);
  //image<float> *smooth_g = smooth(g, sigma);
  //image<float> *smooth_b = smooth(b, sigma);
  Mat matSmoothB, matSmoothG, matSmoothR;
  GaussianBlur(vMatImage[0], matSmoothB, Size(0,0), sigma);
  GaussianBlur(vMatImage[1], matSmoothG, Size(0,0), sigma);
  GaussianBlur(vMatImage[2], matSmoothR, Size(0,0), sigma);

  // build graph
  edge *edges = new edge[cols*rows*4];
  int num = 0;
  //for (int y = 0; y < height; y++) {
  for (int r = 0; r < rows; r++) {
  	//for (int x = 0; x < width; x++) {
  	for (int c = 0; c < cols; c++) {

      if (c < cols-1) {
      	edges[num].a = r * cols + c;
				edges[num].b = r * cols + (c+1);
				edges[num].w = diff(matSmoothB, matSmoothG, matSmoothR, c, r, c+1, 1);
				num++;
      }

      if (r < rows-1) {
				edges[num].a = r * cols + c;
				edges[num].b = (r+1) * cols + c;
				edges[num].w = diff(matSmoothB, matSmoothG, matSmoothR, c, r, c, r+1);
				num++;
      }

      if ((c < cols-1) && (r < rows-1)) {
				edges[num].a = r * cols + c;
				edges[num].b = (r+1) * cols + (c+1);
				edges[num].w = diff(matSmoothB, matSmoothG, matSmoothR, c, r, c+1, r+1);
				num++;
      }

      if ((c < cols-1) && (r > 0)) {
				edges[num].a = r * cols + c;
				edges[num].b = (r-1) * cols + (c+1);
				edges[num].w = diff(matSmoothB, matSmoothG, matSmoothR, c, r, c+1, r-1);
				num++;
      }
    }
  }

  // segment
  universe *u = segment_graph(cols*rows, num, edges, k);
  
  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
  }
  delete [] edges;
  *num_ccs = u->num_sets();

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

  delete u;

  return output;
}

#endif
