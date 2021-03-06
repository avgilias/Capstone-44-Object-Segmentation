#include <iostream>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/features/vfh.h>
#include <pcl/features/integral_image_normal.h>
#include <pcl/visualization/cloud_viewer.h>     //Visualization class 1
#include <pcl/visualization/pcl_visualizer.h>   // Visualization class 2
#include <pcl/visualization/histogram_visualizer.h> // histogram visualizer
#include <pcl/filters/voxel_grid.h>  //For voxel grid
#include <pcl/filters/passthrough.h> // For passthrough filter
#include <pcl/console/parse.h>       // To parse Console
#include <pcl/filters/statistical_outlier_removal.h>  // Statistical Filtering
#include <pcl/features/normal_3d.h>

#define PASSTHROUGH_FILTER
//#define VOXEL_FILTER
//#define OUTLIER_REMOVAL   
//#define NORMAL_COMPUTE
#define GENERAL_NORMAL_COMPUTE
//#define VFH_DESCRIPTOR

std::string modelFileName;
//std::string sceneFileName;

//-------------------------------------------------------
// Command line parser 
void 
parseCommandLine (int argc, char *argv[])
{
	// Model & Scene Filenames
	std::vector<int> filenames;
	filenames = pcl::console::parse_file_extension_argument(argc, argv, ".pcd");
	if(filenames.size() !=1)  // For  files set the value to 2
	{
		std::cout << "Filenames missing \n";
		std::cout << "./executable model.pcd scene.pdc \n";
		exit(-1);
	}

	// Store the filenames in temp variables
	modelFileName = argv[filenames[0]];
	//sceneFileName = argv[filenames[1]];
}


int main( int argc, char *argv[])
{ 
	//=================================================================
	// Create a cloud pointer
	// The file should be in the same folder
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
//=================================================================
 	// Parse Command Line
 	parseCommandLine (argc, argv);
 	// Load the file
	if(pcl::io::loadPCDFile(modelFileName, *cloud) < 0)
	{
		std::cout << "Error Loading model cloud." <<std::endl;
		return(-1);
	}
//================================================================================//
	 // It is very important to run pass through filter. it cuts off values 
	 // that are either inside or outside a given user range.
	 // I could not computer Normals directly from Voxal Filter.. As was suggested
	// in community post, this step was essential.
	 //POINT CLOUD---> Flitering---> Pass through filter   
      //a) 
//===============================================================================//
	#ifdef PASSTHROUGH_FILTER	
		pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_pass_filtered (new pcl::PointCloud<pcl::PointXYZ>);

  	// we create the PassThrough filter object, and set its parameters.
	 	pcl::PassThrough<pcl::PointXYZ> pass;
 	 //	pass.setInputCloud (cloud_voxal_filtered);
	 	pass.setInputCloud (cloud);
 		pass.setFilterFieldName ("x");
  		pass.setFilterLimits (0.00, 0.07);
 	   // pass.setFilterLimitsNegative (false);
 		pass.filter (*cloud_pass_filtered);

		cloud = cloud_pass_filtered;   // Set the new cloud 
/*
		//visualize Filtered Data
		pcl::visualization::CloudViewer viewer("PCL Viewer");
		
		viewer.showCloud(cloud);
		while(!viewer.wasStopped())
		{
			
		}  */
    #endif
 	
//==========================================================================
      //POINT CLOUD---> Flitering    
      //a) Voxal Grid  
     //===============================================================================//
      // Create a cloud for voxel filter
    #ifdef VOXEL_FILTER
		std::cerr << "PointCloud before filtering: " << cloud->width * cloud->height 
       << " data points (" << pcl::getFieldsList (*cloud) << ").";

		pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_voxal_filtered (new pcl::PointCloud<pcl::PointXYZ>);

		// a pcl::VoxelGrid filter is created with a leaf size of 1cm,
		// the input data is passed, and the output is computed and stored in cloud_filtered.
		pcl::VoxelGrid<pcl::PointXYZ>sor;
		sor.setInputCloud(cloud);
		sor.setLeafSize(0.01f, 0.01f, 0.01f);
		sor.filter(*cloud_voxal_filtered);

		std::cerr << "PointCloud after filtering: " << cloud_voxal_filtered->width *cloud_voxal_filtered->height 
      	 << " data points (" << pcl::getFieldsList (*cloud_voxal_filtered) << ").";
      	
      	// Set cloud = new cloud voxal filtered
      	 cloud = cloud_voxal_filtered;
   /*   	// Save to a file
     	 pcl::io::savePCDFileASCII ("downsampledModel.pcd",*cloud_voxal_filtered);
		
	
	
		//visualize Filtered Data
		pcl::visualization::CloudViewer viewer("PCL Viewer");
		
		viewer.showCloud(cloud);
		while(!viewer.wasStopped())
		{
			
		}  */

	#endif	


     //================================================================================//
	/* //==========================================================================
      //POINT CLOUD---> Flitering   
	  //*** Note this tool is very tricky. You will have to juggle with fine tuning
	  // The standard deviation and number of points!
      //c) Statistical Outlier Removal is a technique that removes outliers from 
      // an image  
     //===============================================================================//
      // Create a cloud for voxel filter
    #ifdef OUTLIER_REMOVAL
		std::cerr << "PointCloud before OUTLIER filtering: " << cloud->width * cloud->height 
       << " data points (" << pcl::getFieldsList (*cloud) << ").";

		 pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_outlier_removal (new pcl::PointCloud<pcl::PointXYZ>);

		// a pcl::VoxelGrid filter is created with a leaf size of 1cm,
		// the input data is passed, and the output is computed and stored in cloud_filtered.
	// Create the filtering object
 	 	pcl::StatisticalOutlierRemoval<pcl::PointXYZ> outliers;
  		outliers.setInputCloud (cloud);
  		outliers.setMeanK (100);
  		outliers.setStddevMulThresh (0.05);
  		outliers.filter (*cloud_outlier_removal);

		std::cerr << "PointCloud after OUTLIER filtering: " << cloud_outlier_removal->width *cloud_outlier_removal->height 
      	 << " data points (" << pcl::getFieldsList (*cloud_outlier_removal) << ").";
      	
      	// Set cloud = new cloud voxal filtered
      	 cloud = cloud_outlier_removal;
     	// Save to a file
     	// pcl::io::savePCDFileASCII ("downsampledModel.pcd",*cloud_voxal_filtered);
		
	
	
		//visualize Filtered Data
		pcl::visualization::CloudViewer viewer("PCL Viewer");
		
		viewer.showCloud(cloud);
		while(!viewer.wasStopped())
		{
			
		}  

	#endif	

	*/
     //================================================================================//
	 //POINT CLOUD---> Flitering---> Normals    
      //a) 
     //===============================================================================//
	#ifdef NORMAL_COMPUTE	
		//Create an object for the normal estimation and 
		//compute the normals
	//	pcl::io::loadPCDFile("milk_cartoon_downsampled.pcd", *cloud);
		pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
		// The AVERAGE_3D_GRADIENT mode creates 6 integral images to 
		//compute the normals using the cross-product of horizontal and
		//vertical 3D gradients and computes the normals using the cross-
		//product between these two gradients.

		pcl::IntegralImageNormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
             ne.setNormalEstimationMethod (ne.AVERAGE_3D_GRADIENT);
             ne.setMaxDepthChangeFactor(0.02f);
             ne.setNormalSmoothingSize(10.0f);
             ne.setInputCloud(cloud);   		// Load the cloud
             ne.compute(*normals);				// Compute Normal
         //visualize normals
		pcl::visualization::PCLVisualizer viewer("PCL Viewer");
		viewer.setBackgroundColor(0.0,0.0,0.5);
		viewer.addPointCloudNormals<pcl::PointXYZ,pcl::Normal>(cloud,normals);
		while(!viewer.wasStopped())
		{
			viewer.spinOnce();
		}  
    #endif
    //=====================================================================================//  
      //================================================================================//
	 // This normal estimation is for all images--not necessarily organized
     //===============================================================================//
	#ifdef GENERAL_NORMAL_COMPUTE	
		// Output dataset
		std::cerr << "PointCloud Size before Normal: " << cloud->points.size ()<< ".\n";

		pcl::PointCloud<pcl::Normal>::Ptr general_normals(new pcl::PointCloud<pcl::Normal>);
		// Create the normal estimation class, and pass the input dataset to it
		pcl::IntegralImageNormalEstimation<pcl::PointXYZ, pcl::Normal> gne;
		gne.setInputCloud (cloud);
        // Create an empty kdtree representation, and pass it to the normal estimation object.
  		// Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
  		pcl::search::Search<pcl::PointXYZ>::Ptr gtree = boost::shared_ptr<pcl::search::Search<pcl::PointXYZ> > (new pcl::search::KdTree<pcl::PointXYZ>);
  		pcl::PointCloud <pcl::Normal>::Ptr normals (new pcl::PointCloud <pcl::Normal>);
  		pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> normal_estimator;
  		normal_estimator.setSearchMethod (gtree);
  		normal_estimator.setInputCloud (cloud);
  		normal_estimator.setKSearch (50);
  		normal_estimator.compute (*normals);

 		std::cerr << "PointCloud Size after Normal: " <<normals->points.size ()<< ".\n";

    /* 	 //visualize normals
		pcl::visualization::PCLVisualizer viewer("PCL Viewer");
		viewer.setBackgroundColor(0.0,0.0,0.5);
		viewer.addPointCloudNormals<pcl::PointXYZ,pcl::Normal>(cloud,general_normals);
		while(!viewer.wasStopped())
		{
			viewer.spinOnce();
		}  */
    #endif
    //=====================================================================================//      //POINT CLOUD---> Flitering---> Normals----> VFH Extraction    
      //a) 
     //===============================================================================//
     #ifdef VFH_DESCRIPTOR
   		 // Now the normal is computed, we are ready to use VFH
    	 pcl::VFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::VFHSignature308> vfh;
   		 vfh.setInputCloud(cloud_pass_filtered);
     	 vfh.setInputNormals(normals);

    	 // Create an empty kdtree representation, and pass it to the FPFH
    	 //estimation object.
    	 //Its content will be filled inside the object, based on the given input dataset

    	 pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ>());
    	 vfh.setSearchMethod(tree);

     	//Output datasets
     	// Create an empty PointCloud representation, and pass it to vfh
     	 pcl::PointCloud<pcl::VFHSignature308>::Ptr vfhs(new pcl::PointCloud<pcl::VFHSignature308>());

   		 // Normalize bins
     	vfh.setNormalizeBins(true);
    	vfh.setNormalizeDistance(false);
    	// Compute the features
     	 vfh.compute(*vfhs);
     
     	//VFH Visualization.
		pcl::visualization::PCLHistogramVisualizer viewer;

		// Set the size of the descriptor beforehand
		viewer.addFeatureHistogram(*vfhs, 308);

		viewer.spin(); 
	#endif
	//===============================================================================//

	return 0;
}