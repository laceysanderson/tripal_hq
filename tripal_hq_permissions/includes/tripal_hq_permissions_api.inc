<?php

/**
 * Tripal jobs callback
 *
 * @param $args
 */
function tripal_hq_permissions_infer_permissions($args) {

  $uid = $args;

  //Get parent user permissions.

  $parents = tripal_hq_permissions_get_root_permissions($uid);

  //For each permission, find all child records and add them.

  if (!$parents) {
    tripal_log('user !uid had no permissions to set.', TRIPAL_WARNING);
    return;
  }

  //set hq_submission permissions.

  $submissions = db_select('tripal_hq_submission', 't')
    ->fields('t')
    ->execute();

  foreach ($submissions as $submission) {

    $data = unserialize($submission->data);

    tripal_hq_permissions_process_entity_permissions($submission->id, $data);
  }
}

/**
 * @param $uid
 *
 * @return array
 */
function tripal_hq_permissions_get_root_permissions($uid) {


  $permissions = db_select('tripal_hq_permissions', 't')
    ->fields('t')
    ->condition('uid', $uid)
    ->execute()
    ->fetchAll();

  return $permissions;
}

/**
 * Is this necessary?
 * I think not.
 */

//function tripal_hq_permissions_set_child_permissions($parent) {
//  $base = $parent->base_table;
//  $record_id = $parent->record_id;
//
//
//  return TRUE;
//}

/**
 * When a new submission is created, check its fields and see if this falls
 * under a parent permission.  For example, an organism created in a project
 * hwere that project has hq_permissions setup.
 *
 * @param $submission_id
 *
 * @return bool
 */
function tripal_hq_permissions_process_submission($submission_id) {
  $submission = db_select('tripal_hq_submission', 't')
    ->fields('t')
    ->condition('id', $submission_id)
    ->execute()
    ->fetchObject();
  $entity = unserialize($submission->data);


  tripal_hq_permissions_process_entity_permissions($submission_id, $entity);
}


/**
 * For a given record specified via record_id and base table, retrieve all
 * users signed up for permissions for it in the tripal_hq_permissions table.
 * Link this submission to that user via the tripal_hq_submission_permissions
 * table.
 *
 * @param $base - Chado base table.  organism, project.
 * @param $record_id
 * @param $submission_id
 */
function tripal_hq_permissions_link_request($base, $record_id, $submission_id) {

  $permissions = db_select('tripal_hq_permissions', 't')
    ->fields('t', ['id'])
    ->condition('base_table', $base)
    ->condition('record_id', $record_id)
    ->execute()
    ->fetchAll();

  if (!$permissions) {
    return;
  }
  foreach ($permissions as $permission) {

    db_insert('tripal_hq_submission_permissions')
      ->fields([
        'hq_permission_id' => $permission->id,
        'submission_id' => $submission_id,
      ])
      ->execute();
  }
}

/**
 * @param $submission_id
 * @param $entity
 */
function tripal_hq_permissions_process_entity_permissions($submission_id, $entity) {

  $base = $entity->chado_table;
  if ($base == 'organism' || $base == 'project') {
    //do we care if its an organism or project?  I actually dont think so.
  }

  //organism base table field?

  if (isset($entity->obi__organism)) {

    $organisms = $entity->obi__organism;

    foreach ($organisms['und'] as $organism) {

      $organism_id = $organism['value'];

      tripal_hq_permissions_link_request('organism', $organism_id, $submission_id);
    }
  }

  if (isset($entity->obi__organism_linker)) {

    $organisms = $entity->obi__organism_linker;

    foreach ($organisms['und'] as $organism) {

      //field:  see https://github.com/statonlab/tripal_manage_analyses/blob/7ca149463cad59dcb626389dc713cf0bc4a5610e/includes/TripalFields/obi__organism_linker/obi__organism_linker.inc#L173-L184


      $organism_id = $organism['value']['organism_id'];

      tripal_hq_permissions_link_request('organism', $organism_id, $submission_id);

    }
  }
  //TODO: project fields.  THey don't exist yet though i think.
  return;
}


/**
 * @param $record_id
 * @param $record_type
 */
function tripal_hq_permissions_get_admins($record_id, $record_type) {
  $query = db_select('tripal_hq_permissions', 't')
    ->fields('t', ['uid'])
    ->condition('base_table', $record_type)
    ->condition('record_id', $record_id);
  $query->join('users', 'u', 'u.uid = t.uid');
  $query->fields('u', ['mail']);
  $results = $query->execute()
    ->fetchAll();

  $admins = [];

  foreach ($results as $result) {
    $admins[$result->uid] = $result->mail;
  }
  return $admins;
}