#include "gdalraster/gdalraster.h"
#include "gdalwarpmem/gdalwarpmem.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions
#include "gdal_priv.h"
#include <Rcpp.h>
using namespace Rcpp;
using namespace gdalraster;

#include <Rinternals.h>

SEXP isnull(SEXP pointer) {
  return Rf_ScalarLogical(!R_ExternalPtrAddr(pointer));
}

// [[Rcpp::export]]
SEXP GDALOpen_cpp(CharacterVector dsn, IntegerVector sds) {
  GDALDataset       *poDS;
  poDS = (GDALDataset*) gdalH_open_dsn(dsn[0], sds);
  if( poDS == NULL )
  {
    Rprintf("Problem with 'dsn' input: %s\n", dsn[0]);
    Rcpp::stop("Open failed.\n");
  }
  Rcpp::XPtr<GDALDataset> ptr(poDS);
  return ptr;
}
// [[Rcpp::export]]
SEXP xrgdal_Warp(SEXP xp, NumericVector extent, IntegerVector dimension, CharacterVector projection) {
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);

  char** papszArg = nullptr;

  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "MEM");
  papszArg = CSLAddString(papszArg, "-r");

  papszArg = CSLAddString(papszArg, "near");

  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", extent[0]));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", extent[2]));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", extent[1]));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", extent[3]));

  papszArg = CSLAddString(papszArg, "-ts");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", dimension[0]));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", dimension[1]));


  if (!projection[0].empty()){
    // if supplied check that it's valid
    OGRSpatialReference* oTargetSRS = nullptr;
    oTargetSRS = new OGRSpatialReference;
    OGRErr target_chk =  oTargetSRS->SetFromUserInput(projection[0]);
    if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");
    delete oTargetSRS;
    papszArg = CSLAddString(papszArg, "-t_srs");
    papszArg = CSLAddString(papszArg, projection[0]);
  }


  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
  GDALDatasetH *poSrcDS;
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * 1));
  poSrcDS[0] = (GDALDatasetH) ptr;
  GDALDatasetH hRet = GDALWarp( "", nullptr,
                                1, poSrcDS,
                                psOptions, nullptr);


  CPLAssert( hRet != NULL );
  GDALWarpAppOptionsFree(psOptions);

  Rcpp::XPtr<GDALDataset> op((GDALDataset*) hRet);
  return op;
}

// [[Rcpp::export]]
SEXP xrgdal_RasterIO(SEXP xp, SEXP window, IntegerVector bands) {
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);
  std::vector<int> readbands(bands.size());
  for (int i = 0; i < readbands.size(); i++) readbands[i]= bands[i];
  return gdalraster::gdal_read_band_values(ptr, window, readbands, "Float64", "near", false);
}

// [[Rcpp::export]]
SEXP xrgdal_GetRasterCount(SEXP xp) {
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);
  std::vector<int> n = {ptr->GetRasterCount()};
  return Rcpp::wrap(n);
}
// [[Rcpp::export]]
SEXP xrgdal_GetRasterBand(SEXP xp, IntegerVector band) {
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);
  GDALRasterBand * poBand;
  poBand = ptr->GetRasterBand(band[0]);
  Rcpp::XPtr<GDALRasterBand> op(poBand);
  return op;
}

// [[Rcpp::export]]
SEXP xrgdal_GetOverviewCount(SEXP xp) {
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALRasterBand> ptr(xp);
  int no = ptr->GetOverviewCount();

  return IntegerVector::create(no);

}

// [[Rcpp::export]]
SEXP xrgdal_GetBlockSize(SEXP xp) {

  Rcpp::XPtr<GDALRasterBand> ptr(xp);
  int  nx, ny;
  ptr->GetBlockSize(&nx, &ny);
  IntegerVector out(2);
  out[0] = nx;
  out[1] = ny;

  return out;

}
// [[Rcpp::export]]
SEXP xrgdal_RasterSize(SEXP xp) {
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);
  std::vector<int> dim = {ptr->GetRasterXSize(), ptr->GetRasterYSize()};
  return Rcpp::wrap(dim);
}

/// invoke the close method
// [[Rcpp::export]]
SEXP xrgdal_GDALClose(SEXP xp) {
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);
  if (isnull(ptr)) stop("pointer is nil");
  NumericVector res(1);
  res[0] = 0;
  GDALClose(ptr);
  res[0] = 1;
  // return the result to R
  return res;
}
