// marmer
// by y.fujii <y-fujii at mimosa-pudica.net>, public domain

#include <exception>
#include <iostream>
#include <algorithm>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>

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

				if( dx * dx + dy * dy < numeric_limits<double>::min() ) {
					// denormalized number
					dst(x, y)[c] = src(x, y)[c];
				}
				else {
					double dxdy = (
						+ src(x + 1, y + 1)[c]
						- src(x + 1, y - 1)[c]
						- src(x - 1, y + 1)[c]
						+ src(x - 1, y - 1)[c]
					);
					double np = (
						+ (dx * dx) * (src(x, y + 1)[c] + src(x, y - 1)[c])
						+ (dy * dy) * (src(x + 1, y)[c] + src(x - 1, y)[c])
						- dx * dy * dxdy * (1.0 / 2.0)
					) / (dx * dx + dy * dy);
					dst(x, y)[c] = (src(x, y)[c] + np) * (1.0 / 3.0);
				}
			}
		}
	}

	// copy boundary
	for( int x = 0; x < w; ++x ) {
		dst(x, 0) = src(x, 0);
		dst(x, h - 1) = src(x, h - 1);
	}
	for( int y = 1; y < h - 1; ++y ) {
		dst(0, y) = src(0, y);
		dst(w - 1, y) = src(w - 1, y);
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

int main( int argc, char const* const* argv ) {
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
	catch( exception const& ) {
		cout << "Usage: " << argv[0] << " count src dst\n";
		return 1;
	}

	try {
		// read image
		rgb32f_image_t img0;
		png_read_and_convert_image( argv[2], img0 );
		rgb32f_image_t img1( img0.dimensions() );

		// process image
		for( int i = 0; i < count; ++i ) {
			cout << "\riter step: " << i;
			cout.flush();
			minimizeNorm( view( img0 ), view( img1 ) );
			minimizeNorm( view( img1 ), view( img0 ) );
		}
		cout << endl;

		// write image
		rgb8_image_t dst( img0.dimensions() );
		transform_pixels( view( img0 ), view( dst ), Clip() );
		png_write_view( argv[3], view( dst ) );
	}
	catch( exception const& e ) {
		cout << e.what() << endl;
		return 1;
	}

	return 0;
}
