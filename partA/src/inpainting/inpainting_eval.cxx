// 
//  YOU NEED TO MODIFY THIS FILE FOR YOUR INPAINTING
//  IMPLEMENTATION
//
//  DO NOT MODIFY THIS FILE ANYWHERE EXCEPT WHERE 
//  EXPLICITLY NOTED!!!!
//

// See the file inpainting_eval.h for a detailed explanation of
// the input and output parameters of the routines you must 
// implement

#include "inpainting_eval.h"

// When beginning to write your code, I suggest you work in 
// three steps:
// 
//   - First, implement the routine for computing the
//     confidence term. To help with debugging, you should
//     let compute_D() always return 1, so that patches
//     are selected purely based on their confidence term.
//     (correctness of the confidence computation routine
//     should be relatively easy to check since if it 
//     incorrect patches with fewer filled pixels will end up
//     having higher priorities and will be chosen first)
//
//  - Second, implement the routine that does the patch lookup.
//    The correctness of the lookup routine should also be fairly
//    easy to debug, since if it is incorrect the routine will
//    be choosing patches that look nothing like the patch on
//    the fill front.
//    Together, these two steps will allow you to get somewhat
//    reasonable inpaintings done.
//
//  - Third, implement the data term computation routine. You
//    should also try to do this in steps to help with 
//    debugging: (1) make your compute_C() function return 1
//    always, so that patch priorities are computed entirely
//    according to the data term. (2) make your compute_D() 
//    function return just the magnitude of the 
//    gradient computed by the compute_gradient() routine
//    ---this will help you debug gradient computatoins, since an 
//    incorrect computatoin will cause patches that contain 
//    very low intensity variation to be selected before 
//    patches with lots of intensity variations. (3) once
//    you are satisfied that gradient computatoins are 
//    correct, move on to the normal computation routine, etc.
// 
//  - Only when you are satisfied that the above routines
//    are correct individually should you try to compute the
//    priorities as the product C()*D(). Otherwise, if the
//    patch selections 'don't look right' you won't know
//    what is causing this behavior
//
//  

double compute_D(psi& PSI, 
				 const vil_image_view<vxl_byte> im,
				 const vil_image_view<bool> unfilled, 
				 const vil_image_view<bool> fill_front, 
				 double alpha,
				 vnl_double_2& gradient, 
				 vnl_double_2& front_normal)
{

	// holds the perpendicular to the gradient
	vnl_double_2 grad_normal;    

	// compute the gradient at a filled pixel in the 
	// direction of the front normal
	if (compute_gradient(PSI, im, unfilled, gradient)) {

		grad_normal(0) = -gradient(1);
		grad_normal(1) = gradient(0);

		// now compute the normal of the fill front
		if (compute_normal(PSI, fill_front, front_normal)) {
			double dotp;

			dotp = fabs(dot_product(grad_normal, front_normal))/alpha;

			return dotp;
		}
		if (alpha > 0) 
			return 1/alpha;
		else {
			return 0;
		}
	} else {
		// if we cannot compute a normal, the fill boundary consists
		// of exactly one pixel; the data term in this case is meaningless
		// so we just return a default value
		return 0;
	}
}

///////////////////////////////////////////////////////////
//     DO NOT CHANGE ANYTHING ABOVE THIS LINE            //
///////////////////////////////////////////////////////////


//
// YOU NEED TO IMPLEMENT THE ROUTINES BELOW
//



double compute_C(psi& PSI, const vil_image_view<double>& C, 
				 const vil_image_view<bool>& unfilled)
{
	///////////////////////////////////////////////////////////
	//     DO NOT CHANGE ANYTHING ABOVE THIS LINE            //
	///////////////////////////////////////////////////////////
	
	vnl_matrix<double> c_vals;
	vnl_matrix<int> valid, unfilled_pixels, unfilled_valid;
	double sum;
	
	//get the values of images
	PSI.get_pixels(C, c_vals, valid);
	PSI.get_pixels(unfilled, unfilled_pixels, unfilled_valid);
	
	//sets filled pixels to 1, and unfilled to 0
	unfilled_pixels = 1 - unfilled_pixels;
	
	PSI.begin(unfilled_pixels);
	
	while(PSI.next(unfilled_pixels)){
	  int i, j;
	  PSI.psi_coord(i, j);
	  sum += c_vals[i][j];
	}
	
	return sum/(PSI.sz()*PSI.sz());

	///////////////////////////////////////////////////////////
	//     DO NOT CHANGE ANYTHING BELOW THIS LINE            //
	///////////////////////////////////////////////////////////
}


bool compute_normal(psi& PSI,
					vil_image_view<bool> fill_front, 
					vnl_double_2& normal)
{
	///////////////////////////////////////////////////////////
	//     DO NOT CHANGE ANYTHING ABOVE THIS LINE            //
	///////////////////////////////////////////////////////////

	
	///////////////////////////////////////////////////////////
	//              PLACE YOUR CODE HERE                     //
	///////////////////////////////////////////////////////////

        // dummy routine
        normal(0) = 1;
        normal(1) = 0;
        return true;

	///////////////////////////////////////////////////////////
	//     DO NOT CHANGE ANYTHING BELOW THIS LINE            //
	///////////////////////////////////////////////////////////
}

// return the gradient with the strongest magnitude inside the
// patch of radius w or return false if no gradients can be computed
bool compute_gradient(psi& PSI,
					  const vil_image_view<vxl_byte>& inpainted_grayscale, 
					  const vil_image_view<bool>& unfilled, 
					  vnl_double_2& grad)
{
	///////////////////////////////////////////////////////////
	//     DO NOT CHANGE ANYTHING ABOVE THIS LINE            //
	///////////////////////////////////////////////////////////

	vnl_matrix<int> greys, valid, unfilled_pixels, unfilled_valid;
	vnl_matrix<int> smask_x(3,3), smask_y(3,3);
	double mag, largestMag;
	bool hasValidPatch = false;
	largestMag = 0;
	mag =0;
	
	//sobel masks
	smask_x(0,0) = -1; smask_x(1,0) = -2; smask_x(2,0) = -1;
	smask_x(0,1) = 0; smask_x(1,1) = 0; smask_x(2,1) = 0;
	smask_x(0,2) = 1; smask_x(1,2) = 2; smask_x(2,2) = 1;
	
	smask_y = smask_x.transpose();
	
	//setting up the unfilled mask
	PSI.get_pixels(unfilled, unfilled_pixels, unfilled_valid);
	unfilled_pixels = 1 - unfilled_pixels;
	
	//goes through all the nonfilled pixels contained in the patch passed in
	PSI.begin(unfilled_pixels);
	while (PSI.next(unfilled_pixels)) {
	  //calculate the gradient of each valid 3x3 patch within the patch
	  int p, i, j;
	  PSI.image_coord(i, j);
	  
	  //creates a psi for the 3x3 patch centered at the current pixel
	  vnl_double_2 center(i,j);
	  psi patch(center, 1, inpainted_grayscale.ni(), inpainted_grayscale.nj());
	  patch.get_pixels(inpainted_grayscale, greys, valid);
	  
	  //gets the intensities for the 3x3 patch
	  vnl_matrix<int> A, A_valid;
	  patch.get_pixels(inpainted_grayscale, A, A_valid);
	  
	  double gx, gy;
	  int pi, pj;
	  //goes through each of the pixels in the 3x3 to calculate the gradient
	  patch.begin();
	  while(patch.next()) {
	    patch.image_coord(i, j);
	    patch.psi_coord(pi, pj);
	    if (valid[pi][pj] == 0 || unfilled(i,j) == 1) {
	      //pixels is invalid or is unfilled
	      gx = 0;
	      gy = 1;
	      break;
	    }
	    gx += A[pi][pj]*smask_x[pi][pj];
	    gy += A[pi][pj]*smask_y[pi][pj];
	  }
	  
	  mag = sqrt(pow(gx,2) + pow(gy,2));
	  
	  if (mag > largestMag) {
	    hasValidPatch = true;
	    largestMag = mag;
	    grad(0) = gx;
	    grad(1) = gy;
	  }
	}
	return hasValidPatch;

	///////////////////////////////////////////////////////////
	//     DO NOT CHANGE ANYTHING BELOW THIS LINE            //
	///////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////
//     PLACE ANY ADDITIONAL CODE (IF ANY) HERE           //
///////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////

