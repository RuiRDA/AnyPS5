# libSceNKWebKit

## add Functional

- [WTF grapheme cluster counts](../../../core/libs/prx/libSceNKWebKit/src/GraphemeClusters.cpp) (libSceNKWebKit) - console Unicode version and segmentation are unmeasured. The exports follow [PS5 WebKit 11.x TextBreakIterator.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WTF/wtf/text/TextBreakIterator.cpp#L178-L233). UTF-16 segmentation applies the [Unicode 17 extended grapheme rules](https://www.unicode.org/reports/tr29/tr29-47.html#Grapheme_Cluster_Boundary_Rules) using the bundled ICU 78.1 properties. A null buffer the function must read throws `std::invalid_argument`. Closes when: compared with a console module.

## remove | `_ZN3WTF19numGraphemeClustersENS_10StringViewE` | `K7qAbz7P3rs` |

## remove | `_ZN3WTF30numCodeUnitsInGraphemeClustersENS_10StringViewEj` | `3PZOiy2oZ9w` |
