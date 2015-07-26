from skimage.feature import hog
from gtk._gtk import Orientation

def computeHOG(image,orientations=9,pixels_per_cell=(6,6),cells_per_block=(3,3),visualise=False, normalise=True):
#     pixel_x = image.shape[1]/block_count[0];
#     pixel_y = image.shape[0]/block_count[1];
    feat = hog(image, orientations=orientations, pixels_per_cell=pixels_per_cell,cells_per_block=cells_per_block, visualise = visualise, normalise=normalise);
    return feat;
    