This is the modified version of W3C's CSS2.1 test suite.

http://www.w3.org/Style/CSS/Test/CSS2.1/20100127/html4/chapter-13.htm

We maintain the hand-modified version because adding unmodified printing
related tests into LayoutTests/css2.1 directory doesn't make sense at all.

We need the following modifications to add W3C's test into this directory:

- Eliminate tabs.
- Add dumpAsText() call.
- Add test() function and implement it using layoutTestController functions.
- Add id attributes for elements in question.
- Add <div id="results"> to report the test results.

FIXME: We have no way to get the number of last page for now.
       Though page-break-before-003 is actually failing, we cannot add
       failing test due to the lack of this feature.

FIXME: page-margin-* aren't added yet. We may need
       layoutTestController APIs to test margins.
