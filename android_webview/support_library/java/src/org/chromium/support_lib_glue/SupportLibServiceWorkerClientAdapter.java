// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.support_lib_glue;

import android.webkit.WebResourceResponse;

import com.android.webview.chromium.ServiceWorkerClientAdapter;
import com.android.webview.chromium.WebResourceRequestAdapter;

import org.chromium.android_webview.AwContentsClient.AwWebResourceRequest;
import org.chromium.android_webview.AwServiceWorkerClient;
import org.chromium.android_webview.AwWebResourceResponse;
import org.chromium.support_lib_boundary.ServiceWorkerClientBoundaryInterface;

/**
 * Adapter between ServiceWorkerClientBoundaryInterface and AwServiceWorkerClient.
 */
class SupportLibServiceWorkerClientAdapter extends AwServiceWorkerClient {
    ServiceWorkerClientBoundaryInterface mImpl;

    SupportLibServiceWorkerClientAdapter(ServiceWorkerClientBoundaryInterface impl) {
        mImpl = impl;
    }

    @Override
    public AwWebResourceResponse shouldInterceptRequest(AwWebResourceRequest request) {
        WebResourceResponse response =
                mImpl.shouldInterceptRequest(new WebResourceRequestAdapter(request));
        return ServiceWorkerClientAdapter.fromWebResourceResponse(response);
    }
}
