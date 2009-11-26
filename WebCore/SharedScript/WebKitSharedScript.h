/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebKitSharedScript_h
#define WebKitSharedScript_h

#if ENABLE(SHARED_SCRIPT)

#include "ActiveDOMObject.h"
#include "AtomicStringHash.h"
#include "EventListener.h"
#include "EventTarget.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace WebCore {

    class KURL;
    class SharedScriptContext;
    class ScriptExecutionContext;

    class WebKitSharedScript : public RefCounted<WebKitSharedScript>, public ActiveDOMObject, public EventTarget {
    public:
        static PassRefPtr<WebKitSharedScript> create(const String& url, const String& name, ScriptExecutionContext* context, ExceptionCode& ec)
        {
            return adoptRef(new WebKitSharedScript(url, name, context, ec));
        }
        virtual ~WebKitSharedScript();

        // EventTarget APIs
        virtual ScriptExecutionContext* scriptExecutionContext() const { return ActiveDOMObject::scriptExecutionContext(); }
        virtual WebKitSharedScript* toWebKitSharedScript() { return this; }

        DEFINE_ATTRIBUTE_EVENT_LISTENER(error);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(load);

        using RefCounted<WebKitSharedScript>::ref;
        using RefCounted<WebKitSharedScript>::deref;

        SharedScriptContext* context() const { return m_innerContext.get(); }
        void setContext(PassRefPtr<SharedScriptContext> context);

        // When fired, this will set the innerContext into this WebKitSharedScript and dispatch 'load' event.
        void scheduleLoadEvent(PassRefPtr<SharedScriptContext> innerContext);

    private:
        WebKitSharedScript(const String& url, const String& name, ScriptExecutionContext*, ExceptionCode&);

        virtual void refEventTarget() { ref(); }
        virtual void derefEventTarget() { deref(); }
        virtual EventTargetData* eventTargetData() { return &m_eventTargetData; }
        virtual EventTargetData* ensureEventTargetData() { return &m_eventTargetData; }

        EventTargetData m_eventTargetData;
        RefPtr<SharedScriptContext> m_innerContext;
    };

} // namespace WebCore

#endif // ENABLE(SHARED_SCRIPT)

#endif // WebKitSharedScript_h
