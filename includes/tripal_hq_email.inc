<?php

/**
 * @file
 * Mail functions for notifying users and admins of acceptance, submission, etc.
 */

/**
 * Implements hook_mail().
 *
 * @param string $key
 *   One of the following strings:
 *      - user_submit_notice
 *      - admin_submit_notice
 *      - user_accept_notice
 *      - admin_accept_notice
 *      - user_reject_notice
 *      - admin_reject_notice
 *      - user_edit_notice
 *      - admin_edit_notice.
 */
function tripal_hq_mail(string $key, &$message, $params) {
  $language = $message['language'];
  $site_name = variable_get('site_name');

  global $base_url;

  $uid = $params->uid;
  $user_object = user_load($uid);
  $submitter_name = $user_object->name;
  $user_dash = $base_url . '/tripal_hq/bio_data';
  $admin_dash = $base_url . '/tripal_hq/admin';

  $title = $params->title;
  $bundle_id = $params->bundle_id;

  $bundle = tripal_load_bundle_entity(['id' => $bundle_id]);
  $bundle_label = $bundle->label;

  $entity_id = $params->entity_id;
  $link = $base_url . 'bio_data/' . $entity_id;

  switch ($key) {

    /*
     * Acknowledge user has submitted a content request.
     */
    case 'user_submit_notice':

      $message['body'][] = t("Dear !user,", ['!user' => $submitter_name]);
      $message['subject'] = t('!site Content Submission Request Received', ['!site' => $site_name]);
      $message['body'][] = t('You have successfully submitted a request to create the !type record !title .', [
        '!type' => $bundle_label,
        '!title' => $title,
      ]);
      $message['body'][] = t('You can review your request status at !url', [
        '!url' => $user_dash,
      ]);
      break;

    /*
     * Let relevant admins know a content request has been received.
     */
    case 'admin_submit_notice':

      $message['subject'] = t('!site Content Submission Request Received', ['!site' => $site_name], ['langcode' => $language->language]);
      $message['body'][] = t('Dear !site admin,', ['!site' => $site_name], ['langcode' => $language->language]);
      $message['body'][] = t("User !user has submitted the following content creation request:", [
        '!user' => $submitter_name,
      ]);
      $message['body'][] = t("Type: !type", ['!type' => $bundle_label]);
      $message['body'][] = t("Title: !title", [
        '!title' => $title,
      ]);
      $message['body'][] = t("To approve or deny this request, please visit your Tripal HQ content portal, located here: !url", ['!url' => $admin_dash]);

      break;

    /*
     * User's submission has been approved!
     */
    case 'user_accept_notice':

      $message['subject'] = t('!site Content Submission Request Approved', ['!site' => $site_name], ['langcode' => $language->language]);
      $message['body'][] = t("Dear !user,", ['!user' => $submitter_name]);
      $message['body'][] = t('Your !type submission titled: !title, has been approved.', [
        '!type' => $bundle_label,
        '!title' => $title,
      ]);
      $message['body'][] = t('Your newly created content is available here: !url', ['!url' => $link]);

      break;

    /*
     * Admin: submission has been approved!
     */
    case 'admin_accept_notice':

      $message['body'][] = t('Dear !site admin,', ['!site' => $site_name]);
      $message['body'][] = t('The !type submitted by !user, titled: !title , has been approved.', [
        '!type' => $bundle_label,
        '!title' => $title,
        '!user' => $submitter_name,
      ]);
      $message['body'][] = t('The newly created content is available here: !url', ['!url' => $link]);

      return;

      break;

    case 'user_reject_notice':

      $message['subject'] = t('!site Content Submission Request Denied', ['!site' => $site_name], ['langcode' => $language->language]);
      $message['body'][] = t("Dear !user,", ['!user' => $submitter_name]);
      $message['body'][] = t('Your !type submission titled: !title , has been denied.', [
        '!type' => $bundle_label,
        '!title' => $title,
      ]);

      break;

    case 'admin_reject_notice':

      $message['body'][] = t('Dear !site admin,', ['!site' => $site_name]);
      $message['subject'] = t('!site Content Submission Request Denied', ['!site' => $site_name]);
      $message['body'][] = t('The submission !name has been denied.', ['!name' => $title]);
      break;
  }

  $message['body'][] = '<br/>Regards,<br/>The ' . $site_name . ' team';
  $message['headers']['Content-Type'] = 'text/html; charset=UTF-8; format=flowed';
}
