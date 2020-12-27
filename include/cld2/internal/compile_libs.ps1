param(
    [string]
    $CFLAGS,
    [string]
    $CPPFLAGS,
    [string]
    $CXXFLAGS,
    [string]
    $LDFLAGS
)

<#
:: Prerequisites:
:: - MinGW
::       Can be downloaded from: https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds
::           Then select: version (e.g. 8.1.0) > 'threads-win32' > 'seh'
:: - MSYS 1.0
:: Usage example:
:: - Put this Powershell script into the 'cld2\internal' directory
:: - Using Windows Powershell, cd to the 'cld2\internal' directory
:: - Execute:
::       .\compile_libs.bat -CFLAGS "-O2 -m64 -std=c++98" -LDFLAGS "-lstdc++"
:: - Find the compiled libcld2.dll and libcld2_full.dll in the same directory
#>

if (-not $CFLAGS -and -not $CPPFLAGS -and -not $CXXFLAGS) {
    Write-Warning "Warning: None of CFLAGS, CXXFLAGS or CPPFLAGS is set; you probably should enable some options.";
}

if ($CFLAGS) { Write-Output CFLAGS=$CFLAGS };
if ($CPPFLAGS) { Write-Output CPPFLAGS=$CPPFLAGS };
if ($CXXFLAGS) { Write-Output CXXFLAGS=$CXXFLAGS };
if ($LDFLAGS) { Write-Output LDFLAGS=$LDFLAGS };

$cmd = "g++"
$params = "$CFLAGS $CPPFLAGS $CXXFLAGS -shared -fPIC " +
  "cldutil.cc cldutil_shared.cc compact_lang_det.cc compact_lang_det_hint_code.cc " +
  "compact_lang_det_impl.cc debug.cc fixunicodevalue.cc " +
  "generated_entities.cc generated_language.cc generated_ulscript.cc " +
  "getonescriptspan.cc lang_script.cc offsetmap.cc scoreonescriptspan.cc " +
  "tote.cc utf8statetable.cc " +
  "cld_generated_cjk_uni_prop_80.cc cld2_generated_cjk_compatible.cc " +
  "cld_generated_cjk_delta_bi_4.cc generated_distinct_bi_0.cc " +
  "cld2_generated_quadchrome_2.cc cld2_generated_deltaoctachrome.cc " +
  "cld2_generated_distinctoctachrome.cc  cld_generated_score_quad_octa_2.cc " +
  "-o libcld2.dll $LDFLAGS -Wl,-soname=libcld2.dll" -split '\s+'

& $cmd $params

$params = "$CFLAGS $CPPFLAGS $CXXFLAGS -shared -fPIC " +
  "cldutil.cc cldutil_shared.cc compact_lang_det.cc compact_lang_det_hint_code.cc " +
  "compact_lang_det_impl.cc  debug.cc fixunicodevalue.cc " +
  "generated_entities.cc  generated_language.cc generated_ulscript.cc " +
  "getonescriptspan.cc lang_script.cc offsetmap.cc  scoreonescriptspan.cc " +
  "tote.cc utf8statetable.cc " +
  "cld_generated_cjk_uni_prop_80.cc cld2_generated_cjk_compatible.cc " +
  "cld_generated_cjk_delta_bi_32.cc generated_distinct_bi_0.cc " +
  "cld2_generated_quad0122.cc cld2_generated_deltaocta0122.cc " +
  "cld2_generated_distinctocta0122.cc  cld_generated_score_quad_octa_0122.cc " +
  "-o libcld2_full.dll $LDFLAGS -Wl,-soname=libcld2_full.dll" -split '\s+'

& $cmd $params