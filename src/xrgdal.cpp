#include <cpp11.hpp>
using namespace cpp11;
#include "gdal_priv.h"

void check_alive(SEXP ptr) {
  if( R_ExternalPtrAddr(ptr) == NULL )
  {
    cpp11::stop("external pointer is null\n");
  }

}

[[cpp11::register]]
SEXP gm_GDALOpenShared(r_string dsn) {
  GDALAllRegister();
  // create pointer to an GDAL object and
  // wrap it as an external pointer
  GDALDataset       *poDS;
//  const char* str  = (const char*)dsn[0];
  poDS = (GDALDataset*) GDALOpenShared(&dsn, GA_ReadOnly);

  if( poDS == NULL )
  {
    Rprintf("Problem with 'dsn' input: %s\n", dsn[0]);
    cpp11::stop("Open failed.\n");
  }
  cpp11::external_pointer<GDALDataset> ptr(poDS);
//  Rcpp::XPtr<GDALDataset> ptr(poDS);
  return ptr;
}
[[cpp11::register]]
integers gm_GDALClose(SEXP xp) {
  cpp11::external_pointer<GDALDataset> poDS(xp);
//  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);
  GDALClose(poDS);
  return 0;
}
[[cpp11::register]]
strings gm_GetDescription(SEXP xp) {
  cpp11::external_pointer<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->GetDescription();
}


[[cpp11::register]]
integers gm_GetRasterXSize(SEXP xp) {
  cpp11::external_pointer<GDALDataset> poDS(xp);
  check_alive(poDS);
  cpp11::writable::integers out(1);
  out[0] = poDS->GetRasterXSize();
  return out;
}
