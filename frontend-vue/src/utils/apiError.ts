import { ApiError } from '../api';

type ValidationIssue = {
    loc?: Array<string | number>;
    msg?: string;
};

const REQUEST_SCOPES = new Set(['body', 'query', 'path', 'header', 'cookie']);

const isRecord = (value: unknown): value is Record<string, unknown> => {
    return typeof value === 'object' && value !== null;
};

const isValidationIssue = (value: unknown): value is ValidationIssue => {
    if (!isRecord(value)) {
        return false;
    }

    return Array.isArray(value.loc) && typeof value.msg === 'string' && value.msg.trim().length > 0;
};

const formatLocation = (loc: Array<string | number>): string => {
    const fieldPath = loc
        .map(String)
        .filter((part) => !REQUEST_SCOPES.has(part))
        .join('.');

    return fieldPath || 'request';
};

const formatValidationDetails = (detail: Array<ValidationIssue>): string => {
    return detail
        .map((issue) => `${formatLocation(issue.loc ?? [])}: ${(issue.msg ?? 'Invalid value').trim()}`)
        .join('; ');
};

const parseApiErrorBodyMessage = (body: unknown): string | undefined => {
    if (typeof body === 'string' && body.trim().length > 0) {
        return body;
    }

    if (!isRecord(body)) {
        return undefined;
    }

    const detail = body.detail;
    if (typeof detail === 'string' && detail.trim().length > 0) {
        return detail;
    }

    if (Array.isArray(detail)) {
        const validationIssues = detail.filter(isValidationIssue);
        if (validationIssues.length > 0) {
            return formatValidationDetails(validationIssues);
        }
    }

    const message = body.message;
    if (typeof message === 'string' && message.trim().length > 0) {
        return message;
    }

    return undefined;
};

export const getApiErrorMessage = (error: unknown, fallback: string): string => {
    if (error instanceof ApiError) {
        return parseApiErrorBodyMessage(error.body) ?? error.message ?? fallback;
    }

    if (error instanceof Error && error.message.trim().length > 0) {
        return error.message;
    }

    return fallback;
};
