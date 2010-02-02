// marmer
// by y.fujii <y-fujii at mimosa-pudica.net>, public domain

#include <exception>
#include <iostream>
#include <cmath>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>

using namespace std;


template<class SrcView, class DstView>
void minimizeNorm( SrcView const& src, DstView& dst ) {
	using namespace boost::gil;
	assert( src.dimensions() == dst.dimensions() );

	int w = dst.width();
	int h = dst.height();

	for( int y = 1; y < h - 1; ++y ) {
		for( int x = 1; x < w - 1; ++x ) {
			for( int c = 0; c < num_channels<DstView>::value; ++c ) {
				double dx = (src(x + 1, y)[c] - src(x - 1, y)[c]);
				double dy = (src(x, y + 1)[c] - src(x, y - 1)[c]);

				if( dx * dx + dy * dy == 0.0 ) {
					dst(x, y)[c] = src(x, y)[c];
				}
				else {
					double dxdy = (
						+ src(x + 1, y + 1)[c]
						- src(x + 1, y - 1)[c]
						- src(x - 1, y + 1)[c]
						+ src(x - 1, y - 1)[c]
					);
					dst(x, y)[c] = (
						+ (dx * dx) * (src(x, y + 1)[c] + src(x, y - 1)[c])
						+ (dy * dy) * (src(x + 1, y)[c] + src(x - 1, y)[c])
						- dx * dy * dxdy * (1.0 / 2.0)
					) / ((dx * dx + dy * dy) * 2.0);
				}
			}
		}
	}

	for( int x = 0; x < w; ++x ) {
		dst(x, 0) = src(x, 0);
		dst(x, h) = src(x, h);
	}
	for( int y = 1; y < h - 1; ++y ) {
		dst(0, y) = src(0, y);
		dst(w, y) = src(w, y);
	}
}

struct Clip {
	boost::gil::rgb8_pixel_t operator()( boost::gil::rgb32f_pixel_t src ) {
		return boost::gil::rgb8_pixel_t(
			min( max( int( src[0] * 255.0f + 0.5f ), 0 ), 255 ),
			min( max( int( src[1] * 255.0f + 0.5f ), 0 ), 255 ),
			min( max( int( src[2] * 255.0f + 0.5f ), 0 ), 255 )
		);
	}
};

int main( int argc, char** argv ) {
	using namespace boost::gil;

	// parse args
	int count;
	try {
		if( argc != 4 ) {
			throw exception();
		}
		if( (istringstream( argv[1] ) >> count).fail() ) {
			throw exception();
		}
	}
	catch( exception ) {
		cout << "Usage: " << argv[0] << " count src dst\n";
		return 1;
	}

	// read image
	rgb32f_image_t img0;
	try {
		png_read_and_convert_image( argv[2], img0 );
	}
	catch( exception ) {
		try {
			jpeg_read_and_convert_image( argv[2], img0 );
		}
		catch( exception& e ) {
			cout << e.what() << "\n";
			return 1;
		}
	}
	rgb32f_image_t img1( img0.dimensions() );

	// process image
	for( int i = 0; i < count; ++i ) {
		cout << "\riter step: " << i * 2;
		cout.flush();
		minimizeNorm( view( img0 ), view( img1 ) );
		minimizeNorm( view( img1 ), view( img0 ) );
	}
	cout << endl;

	// write image
	rgb8_image_t dst( img0.dimensions() );
	transform_pixels( view( img0 ), view( dst ), Clip() );
	png_write_view( argv[3], view( dst ) );

	return 0;
}
