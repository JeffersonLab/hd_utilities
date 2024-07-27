from matplotlib import pyplot as plt
from matplotlib.ticker import MultipleLocator, FormatStrFormatter 
import skimage as ski
from skimage.measure import profile_line
from skimage.io import imread

filename = '20231031_CompareOldLMS_1'


#image = imread('test_sanding.jpg')
#image = imread('best_polishing.jpg')
#image = imread('1000grit_noHoles_allWet_oneSideLight_resanded_5000oneside.jpg')
#image = imread('1000grit_noHoles_allWet_twoSideLight_resanded_5000oneside.jpg')
image = imread(filename + '.jpg')


##### SETUP #####
nx = 14
ny = 32
#################



sizeX = round(image.shape[0]/nx)
sizeY = round(image.shape[1]/ny)

print(sizeX,sizeY,image.shape)

plt.figure(100)
fig, ax = plt.subplots(nx,ny, figsize=(34,36), sharex=True, sharey=True)
plt.subplots_adjust(wspace=0, hspace=0)

int_arr = [[0 for y in range(ny)] for x in range(nx)] 
image_gray = ski.color.rgb2gray(image)

for i in range(0,nx):
	for j in range(0,ny):
		image_oneBlock = image_gray[i*sizeX:i*sizeX+sizeX,j*sizeY:j*sizeY+sizeY]
		print(i*sizeX,i*sizeX+sizeX,j*sizeY,j*sizeY+sizeY,image_oneBlock.mean())
		ax[i,j].text(sizeY/4,sizeX/2,round(image_oneBlock.mean(),3))
		int_arr[i][j] = image_oneBlock.mean()
#		ax[i,j].imshow(image_oneBlock, cmap=plt.cm.gray,vmin=0, vmax=1)

plt.figure(200)
fig2, ax2 = plt.subplots(2,1, figsize=(15,15), sharex=True)

ax2[0].imshow(int_arr, cmap=plt.cm.gray,vmin=0, vmax=1)

for i in range(0,nx):
	label_str = "Row "+ str(i)
	print(label_str)
	ax2[1].plot(int_arr[i],label=label_str)

ax2[1].grid()
ax2[1].set_ylim((0,1))
ax2[1].legend()
fig.suptitle(filename)
fig2.suptitle(filename)
fig.savefig(filename + "_RAW.png")
fig2.savefig(filename + "_PROFILE.png")

#plt.tight_layout()
plt.show();
