// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Adds listeners that are used to handle forms, enabling autofill
 * and the replacement method to dismiss the keyboard needed because of the
 * Autofill keyboard accessory.
 * Contains method needed to access the forms and their elements.
 */

goog.provide('__crWeb.form');

/**
 * Namespace for this file. It depends on |__gCrWeb| having already been
 * injected. String 'form' is used in |__gCrWeb['form']| as it needs to be
 * accessed in Objective-C code.
 */
__gCrWeb.form = {};

// Store common namespace object in a global __gCrWeb object referenced by a
// string, so it does not get renamed by closure compiler during the
// minification.
__gCrWeb['form'] = __gCrWeb.form;

/** Beginning of anonymous object */
(function() {
// Skip iframes that have different origins from the main frame. For such
// frames no form related actions (eg. filling, saving) are supported.
try {
  // The following line generates exception for iframes that have different
  // origin that.
  // TODO(crbug.com/792642): implement sending messages instead of using
  // window.top, when messaging framework is ready.
  if (!window.top.document) return;
} catch (error) {
  return;
}

/**
 * Prefix used in references to form elements that have no 'id' or 'name'
 */
__gCrWeb.form.kNamelessFormIDPrefix = 'gChrome~form~';

/**
 * Prefix used in references to field elements that have no 'id' or 'name' but
 * are included in a form.
 */
__gCrWeb.form.kNamelessFieldIDPrefix = 'gChrome~field~';

/**
 * The interval watching for form changes.
 */
__gCrWeb.form.formWatcherInterval = null;

/**
 * The value of the form signature last time formWatcherInterval was
 * triggerred.
 */
__gCrWeb.form.lastFormSignature = {};

/**
 * Based on Element::isFormControlElement() (WebKit)
 * @param {Element} element A DOM element.
 * @return {boolean} true if the |element| is a form control element.
 */
__gCrWeb.form.isFormControlElement = function(element) {
  var tagName = element.tagName;
  return (
      tagName === 'INPUT' || tagName === 'SELECT' || tagName === 'TEXTAREA');
};

/**
 * Returns an array of control elements in a form.
 *
 * This method is based on the logic in method
 *     void WebFormElement::getFormControlElements(
 *         WebVector<WebFormControlElement>&) const
 * in chromium/src/third_party/WebKit/Source/WebKit/chromium/src/
 * WebFormElement.cpp.
 *
 * @param {Element} form A form element for which the control elements are
 *   returned.
 * @return {Array<FormControlElement>}
 */
__gCrWeb.form.getFormControlElements = function(form) {
  if (!form) {
    return [];
  }
  var results = [];
  // Get input and select elements from form.elements.
  // According to
  // http://www.w3.org/TR/2011/WD-html5-20110525/forms.html, form.elements are
  // the "listed elements whose form owner is the form element, with the
  // exception of input elements whose type attribute is in the Image Button
  // state, which must, for historical reasons, be excluded from this
  // particular collection." In WebFormElement.cpp, this method is implemented
  // by returning elements in form's associated elements that have tag 'INPUT'
  // or 'SELECT'. Check if input Image Buttons are excluded in that
  // implementation. Note for Autofill, as input Image Button is not
  // considered as autofillable elements, there is no impact on Autofill
  // feature.
  var elements = form.elements;
  for (var i = 0; i < elements.length; i++) {
    if (__gCrWeb.form.isFormControlElement(elements[i])) {
      results.push(/** @type {FormControlElement} */ (elements[i]));
    }
  }
  return results;
};

/**
 * Returns the field's |id| attribute if not space only; otherwise the
 * form's |name| attribute if the field is part of a form. Otherwhise,
 * generate a technical identifier
 *
 * It is the identifier that should be used for the specified |element| when
 * storing Autofill data. This identifier will be used when filling the field
 * to lookup this field. The pair (getFormIdentifier, getFieldIdentifier) must
 * be unique on the page.
 * The following elements are considered to generate the identifier:
 * - the id of the element
 * - the name of the element if the element is part of a form
 * - the order of the element in the form if the element is part of the form.
 * - generate a xpath to the element and use it as an ID.
 *
 * Note: if this method returns '', the field will not be accessible and
 * cannot be autofilled.
 *
 * It aims to provide the logic in
 *     WebString nameForAutofill() const;
 * in chromium/src/third_party/WebKit/Source/WebKit/chromium/public/
 *  WebFormControlElement.h
 *
 * @param {Element} element An element of which the name for Autofill will be
 *     returned.
 * @return {string} the name for Autofill.
 */
__gCrWeb.form.getFieldIdentifier = function(element) {
  if (!element) {
    return '';
  }
  var trimmedIdentifier = element.id;
  if (trimmedIdentifier) {
    return __gCrWeb.common.trim(trimmedIdentifier);
  }
  if (element.form) {
    // The name of an element is only relevant as an identifier if the element
    // is part of a form.
    trimmedIdentifier = element.name;
    if (trimmedIdentifier) {
      trimmedIdentifier = __gCrWeb.common.trim(trimmedIdentifier);
      if (trimmedIdentifier.length > 0) {
        return trimmedIdentifier;
      }
    }

    var elements = __gCrWeb.form.getFormControlElements(element.form);
    for (var index = 0; index < elements.length; index++) {
      if (elements[index] === element) {
        return __gCrWeb.form.kNamelessFieldIDPrefix + index;
      }
    }
  }
  // Element is not part of a form and has no name or id, or usable attribute.
  // As best effort, try to find the closest ancestor with an id, then
  // check the index of the element in the descendants of the ancestors with
  // the same type.
  var ancestor = element.parentNode;
  while (!!ancestor && ancestor.nodeType == Node.ELEMENT_NODE &&
         (!ancestor.hasAttribute('id') ||
          __gCrWeb.common.trim(ancestor.id) == '')) {
    ancestor = ancestor.parentNode;
  }
  var query = element.tagName;
  var ancestor_id = '';
  if (!ancestor || ancestor.nodeType != Node.ELEMENT_NODE) {
    ancestor = document.body;
  }
  if (ancestor.hasAttribute('id')) {
    ancestor_id = '#' + __gCrWeb.common.trim(ancestor.id);
  }
  var descendants = ancestor.querySelectorAll(element.tagName);
  var i = 0;
  for (i = 0; i < descendants.length; i++) {
    if (descendants[i] === element) {
      return __gCrWeb.form.kNamelessFieldIDPrefix + ancestor_id + '~' +
          element.tagName + '~' + i;
    }
  }

  return '';
};

/**
 * Returns the field's |name| attribute if not space only; otherwise the
 * field's |id| attribute.
 *
 * The name will be used as a hint to infer the autofill type of the field.
 *
 * It aims to provide the logic in
 *     WebString nameForAutofill() const;
 * in chromium/src/third_party/WebKit/Source/WebKit/chromium/public/
 *  WebFormControlElement.h
 *
 * @param {Element} element An element of which the name for Autofill will be
 *     returned.
 * @return {string} the name for Autofill.
 */
__gCrWeb.form.getFieldName = function(element) {
  if (!element) {
    return '';
  }
  var trimmedName = element.name;
  if (trimmedName) {
    trimmedName = __gCrWeb.common.trim(trimmedName);
    if (trimmedName.length > 0) {
      return trimmedName;
    }
  }
  trimmedName = element.id;
  if (trimmedName) {
    return __gCrWeb.common.trim(trimmedName);
  }
  return '';
};

/**
 * Returns the form's |name| attribute if non-empty; otherwise the form's |id|
 * attribute, or the index of the form (with prefix) in document.forms.
 *
 * It is partially based on the logic in
 *     const string16 GetFormIdentifier(const blink::WebFormElement& form)
 * in chromium/src/components/autofill/renderer/form_autofill_util.h.
 *
 * @param {Element} form An element for which the identifier is returned.
 * @return {string} a string that represents the element's identifier.
 */
__gCrWeb.form.getFormIdentifier = function(form) {
  if (!form) return '';
  var name = form.getAttribute('name');
  if (name && name.length != 0 &&
      form.ownerDocument.forms.namedItem(name) === form) {
    return name;
  }
  name = form.getAttribute('id');
  if (name) {
    return name;
  }
  // A form name must be supplied, because the element will later need to be
  // identified from the name. A last resort is to take the index number of
  // the form in document.forms. ids are not supposed to begin with digits (by
  // HTML 4 spec) so this is unlikely to match a true id.
  for (var idx = 0; idx != document.forms.length; idx++) {
    if (document.forms[idx] == form) {
      return __gCrWeb.form.kNamelessFormIDPrefix + idx;
    }
  }
  return '';
};

/**
 * Returns the form element from an ID obtained from getFormIdentifier.
 *
 * This works on a 'best effort' basis since DOM changes can always change the
 * actual element that the ID refers to.
 *
 * @param {string} name An ID string obtained via getFormIdentifier.
 * @return {HTMLFormElement} The original form element, if it can be determined.
 */
__gCrWeb.form.getFormElementFromIdentifier = function(name) {
  // First attempt is from the name / id supplied.
  var form = document.forms.namedItem(name);
  if (form) {
    if (form.nodeType !== Node.ELEMENT_NODE) return null;
    return (form);
  }
  // Second attempt is from the prefixed index position of the form in
  // document.forms.
  if (name.indexOf(__gCrWeb.form.kNamelessFormIDPrefix) == 0) {
    var nameAsInteger =
        0 | name.substring(__gCrWeb.form.kNamelessFormIDPrefix.length);
    if (__gCrWeb.form.kNamelessFormIDPrefix + nameAsInteger == name &&
        nameAsInteger < document.forms.length) {
      return document.forms[nameAsInteger];
    }
  }
  return null;
};



/**
 * Focus and input events for form elements are messaged to the main
 * application for broadcast to WebStateObservers.
 * This is done with a single event handler for each type being added to the
 * main document element which checks the source element of the event; this
 * is much easier to manage than adding handlers to individual elements.
 * @private
 */
var formActivity_ = function(evt) {
  var srcElement = evt.srcElement;
  var value = srcElement.value || '';
  var fieldType = srcElement.type || '';

  var msg = {
    'command': 'form.activity',
    'formName': __gCrWeb.form.getFormIdentifier(evt.srcElement.form),
    'fieldName': __gCrWeb.form.getFieldName(srcElement),
    'fieldIdentifier': __gCrWeb.form.getFieldIdentifier(srcElement),
    'fieldType': fieldType,
    'type': evt.type,
    'value': value
  };
  __gCrWeb.message.invokeOnHost(msg);
};

/**
 * Focus events performed on the 'capture' phase otherwise they are often
 * not received.
 */
document.addEventListener('focus', formActivity_, true);
document.addEventListener('blur', formActivity_, true);
document.addEventListener('change', formActivity_, true);

/**
 * Text input is watched at the bubbling phase as this seems adequate in
 * practice and it is less obtrusive to page scripts than capture phase.
 */
document.addEventListener('input', formActivity_, false);
document.addEventListener('keyup', formActivity_, false);

/**
 * Capture form submit actions.
 */
document.addEventListener('submit', function(evt) {
  var action;
  if (evt['defaultPrevented']) return;
  action = evt.target.getAttribute('action');
  // Default action is to re-submit to same page.
  if (!action) {
    action = document.location.href;
  }
  __gCrWeb.message.invokeOnHost({
    'command': 'document.submit',
    'formName': __gCrWeb.form.getFormIdentifier(evt.srcElement),
    'href': getFullyQualifiedUrl_(action)
  });
}, false);

/** @private
 * @param {string} originalURL
 * @return {string}
 */
var getFullyQualifiedUrl_ = function(originalURL) {
  // A dummy anchor (never added to the document) is used to obtain the
  // fully-qualified URL of |originalURL|.
  var anchor = document.createElement('a');
  anchor.href = originalURL;
  return anchor.href;
};

/**
 * Returns a simple signature of the form content of the page. Must be fast
 * as it is called regularly.
 */
var getFormSignature_ = function() {
  return {
    forms: document.forms.length,
    input: document.getElementsByTagName('input').length
  };
};

/**
 * Install a watcher to check the form changes. Delay is the interval between
 * checks in milliseconds.
 */
__gCrWeb.form['trackFormUpdates'] = function(delay) {
  if (__gCrWeb.form.formWatcherInterval) {
    clearInterval(__gCrWeb.form.formWatcherInterval);
    __gCrWeb.form.formWatcherInterval = null;
  }
  if (delay) {
    __gCrWeb.form.lastFormSignature = getFormSignature_();
    __gCrWeb.form.formWatcherInterval = setInterval(function() {
      var signature = getFormSignature_();
      var old_signature = __gCrWeb.form.lastFormSignature;
      if (signature.forms != old_signature.forms ||
          signature.input != old_signature.input) {
        var msg = {
          'command': 'form.activity',
          'formName': '',
          'fieldName': '',
          'fieldIdentifier': '',
          'fieldType': '',
          'type': 'form_changed',
          'value': ''
        };
        __gCrWeb.form.lastFormSignature = signature;
        __gCrWeb.message.invokeOnHost(msg);
      }
    }, delay);
  }
};

/** Flush the message queue. */
if (__gCrWeb.message) {
  __gCrWeb.message.invokeQueues();
}

}());  // End of anonymous object
